
#include <stdexcept>
#include "Poco/SHA1Engine.h"
#include "Poco/Timestamp.h"
#include "configuration.h"
#include "file.h"

using namespace std;

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
    
    deleteTime=chrono::system_clock::now();
    deleteTime+=chrono::seconds(cfg.keepTime);
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
    //This is the last created file, must be the last to be deleted
    filesInDeleteOrder.push_back(result);
    c.notify_one();
    return result;
}

void FileMap::remove(const string& linkKey)
{
    std::lock_guard<std::mutex> l(m);
    for(auto it=begin(filesInDeleteOrder);it!=end(filesInDeleteOrder);++it)
    {
        if((*it)->getLinkKey()!=linkKey) continue;
        filesInDeleteOrder.erase(it);
        break;
    }
    fm.erase(linkKey);
}

shared_ptr<UploadedFile> FileMap::get(const string& linkKey)
{
    std::lock_guard<std::mutex> l(m);
    return fm[linkKey];
}

FileMap::FileMap()
{
    thread t(&FileMap::fileGarbageCollector,this);
    t.detach();
}

void FileMap::fileGarbageCollector()
{
    for(;;)
    {
        time_point wakeup;
        {
            unique_lock<mutex> l(m);
            while(filesInDeleteOrder.empty()) c.wait(l);
            wakeup=filesInDeleteOrder.front()->getDeleteTime();
        }
        //Wait with the mutex unlocked
        this_thread::sleep_until(wakeup);
        {
            unique_lock<mutex> l(m);
            if(filesInDeleteOrder.empty()) continue;
            //Note that if the file is being downloaded while we remove it
            //nothing bad happens thanks to the magic of the reference counting,
            //it will simply be deleted when the download code, that holds the
            //last reference will end. Easy
            fm.erase(filesInDeleteOrder.front()->getLinkKey());
            filesInDeleteOrder.pop_front();
        }
    }
}
