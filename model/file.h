
#ifndef FILE_H
#define FILE_H

#include <map>
#include <list>
#include <string>
#include <utility>
#include <memory>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "Poco/TemporaryFile.h"
#include "Poco/MD5Engine.h"

typedef std::chrono::time_point
    <std::chrono::system_clock,std::chrono::nanoseconds> time_point;

class UploadedFile
{
public:
    UploadedFile(const std::string& clientFileName,
                 const std::string& mimeType);
    
    std::string getClientFileName() const { return clientFileName; }
    
    std::string getServerFileName() const { return file.path(); }
    
    std::string getMimeType() const { return mimeType; }
    
    std::string getLinkKey() const { return linkKey; }
    
    time_point getDeleteTime() const { return deleteTime; }
    
private:
    std::string clientFileName; ///< Name coming from client
    std::string mimeType;       ///< Mime type
    std::string linkKey;        ///< Key used to retrieve file
    Poco::TemporaryFile file;   ///< Tempfile is deleted by this class dtor
    time_point deleteTime;      ///< Time when file needs to be deleted
};

class FileMap
{
public:
    static FileMap& instance();
    
    std::shared_ptr<UploadedFile> add(const std::string& fileName, const std::string& mimeType);
    
    void remove(const std::string& linkKey);
    
    std::shared_ptr<UploadedFile> get(const std::string& linkKey);
        
private:
    FileMap();
    FileMap(const FileMap&);
    FileMap& operator=(const FileMap&);
    
    void fileGarbageCollector();
    
    std::map<std::string, std::shared_ptr<UploadedFile>> fm;
    std::list<std::shared_ptr<UploadedFile>> filesInDeleteOrder;
    std::mutex m;
    std::condition_variable c;
};

#endif //FILE_H
