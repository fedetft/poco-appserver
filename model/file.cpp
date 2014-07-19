
#include <iostream>
#include <stdexcept>
#include "Poco/SHA1Engine.h"
#include "configuration.h"
#include "file.h"

using namespace std;

//
// class DeferredAction
//

DeferredAction& DeferredAction::instance()
{
    static DeferredAction singleton;
    return singleton;
}

void DeferredAction::enqueue(time_point when, function<void ()> what)
{
    unique_lock<mutex> l(m);
    actions.push(Elem(when,what));
    c.notify_one();
}

void DeferredAction::actionThread()
{
    for(;;)
    {
        time_point wakeup;
        {
            unique_lock<mutex> l(m);
            while(actions.empty()) c.wait(l);
            wakeup=actions.top().when;
        }
        //Wait with the mutex unlocked
        //FIXME: what if an earlier callback arrives while waiting? Some actions
        //may be executed a long time after their desired time
        this_thread::sleep_until(wakeup);
        for(;;)
        {
            auto now=chrono::system_clock::now();
            function<void ()> f;
            {
                unique_lock<mutex> l(m);
                if(actions.empty() || actions.top().when>now) break;
                f=actions.top().what;
                actions.pop();
            }
            //Call the callback with the mutex unlocked
            f();
        }
    }
}

DeferredAction::DeferredAction()
{
    thread t(&DeferredAction::actionThread,this);
    t.detach();
}

//
// class UploadedFile
//

UploadedFile::UploadedFile(const string& clientFileName, const string& mimeType)
    : clientFileName(clientFileName), mimeType(mimeType)
{
    Configuration& cfg=Configuration::instance();
    Poco::SHA1Engine sha1;
    auto tt=chrono::system_clock::now().time_since_epoch().count();
    sha1.update(getServerFileName()+to_string(tt)+cfg.salt);
    linkKey=Poco::DigestEngine::digestToHex(sha1.digest());
}

//
// class FileMap
//

FileMap& FileMap::instance()
{
    static FileMap singleton;
    return singleton;
}

shared_ptr<UploadedFile> FileMap::add(const string& fileName, const string& mimeType)
{
    std::lock_guard<std::mutex> l(m);
    Configuration& cfg=Configuration::instance();
    if(fm.size()>=cfg.maxFiles) throw runtime_error("Too many files");
    shared_ptr<UploadedFile> result(new UploadedFile(fileName,mimeType));
    fm[result->getLinkKey()]=result;
    DeferredAction& da=DeferredAction::instance();
    da.enqueue(chrono::system_clock::now()+chrono::seconds(cfg.keepTime),
               bind(&FileMap::remove,this,result->getLinkKey()));
    
    return result;
}

void FileMap::remove(const string& linkKey)
{
    std::lock_guard<std::mutex> l(m);
    fm.erase(linkKey);
}

shared_ptr<UploadedFile> FileMap::get(const string& linkKey)
{
    std::lock_guard<std::mutex> l(m);
    auto it=fm.find(linkKey);
    if(it==fm.end()) return shared_ptr<UploadedFile>(); //Points to null
    return it->second;
}

FileMap::FileMap() {}

//
// class UserLogin
//

UserLogin& UserLogin::instance()
{
    static UserLogin singleton;
    return singleton;
}

string UserLogin::newUser()
{
    std::lock_guard<std::mutex> l(m);
    Configuration& cfg=Configuration::instance();
    if(logged.size()>=cfg.maxUsers) throw runtime_error("Too many users");
    Poco::SHA1Engine sha1;
    auto now=chrono::system_clock::now();
    sha1.update(to_string(now.time_since_epoch().count())+cfg.salt);
    string result=Poco::DigestEngine::digestToHex(sha1.digest());
    logged.insert(result);
    DeferredAction& da=DeferredAction::instance();
    da.enqueue(now+chrono::seconds(cfg.keepTime),bind(&UserLogin::remove,this,result));
    return result;
}

void UserLogin::remove(const string& cookie)
{
    std::lock_guard<std::mutex> l(m);
    logged.erase(cookie);
}

bool UserLogin::isLoggedIn(const string& cookie)
{
    std::lock_guard<std::mutex> l(m);
    return logged.count(cookie)==1;
}

UserLogin::UserLogin() {}
