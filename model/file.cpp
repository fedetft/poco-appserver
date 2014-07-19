
#include <stdexcept>
#include "Poco/SHA1Engine.h"
#include "Poco/Timestamp.h"
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
    Poco::Timestamp t;
    sha1.update(getServerFileName()+to_string(t.epochMicroseconds())+cfg.salt);
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
    return fm[linkKey];
}

FileMap::FileMap() {}
