
#include <stdexcept>
#include "file.h"

using namespace std;

//
// class UploadedFile
//

UploadedFile::UploadedFile(const string& clientFileName, const string& mimeType)
    : clientFileName(clientFileName), mimeType(mimeType)
{
    Poco::MD5Engine md5;
    md5.update(getServerFileName());
    linkKey=Poco::DigestEngine::digestToHex(md5.digest());
    
    deleteTime=chrono::system_clock::now();
    deleteTime+=chrono::seconds(keepSeconds);
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
    if(fm.size()>=maxFiles) throw runtime_error("Too many files");
    shared_ptr<UploadedFile> result(new UploadedFile(fileName,mimeType));
    fm[result->getLinkKey()]=result;
    //This is the last created file, must be the last to be deleted
    filesInDeleteOrder.push_back(result);
    c.notify_one();
    return result;
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
            //Should never happen but since we've unlocked the mutex...
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
