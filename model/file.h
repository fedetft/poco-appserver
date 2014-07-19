
#ifndef FILE_H
#define FILE_H

#include <map>
#include <list>
#include <vector>
#include <queue>
#include <string>
#include <utility>
#include <memory>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "Poco/TemporaryFile.h"

typedef std::chrono::time_point
    <std::chrono::system_clock,std::chrono::nanoseconds> time_point;

class DeferredAction
{
public:
    static DeferredAction& instance();
    
    /**
     * Enqueue an action to be done at a given time
     * \param when the action won't be done before this time, but may be done
     * after
     * \param what the callback that will be called from a backgroud thread
     */
    void enqueue(time_point when, std::function<void ()> what);
    
private:
    DeferredAction();
    DeferredAction(const DeferredAction&);
    DeferredAction& operator= (const DeferredAction&);
    
    void actionThread();
    
    struct Elem
    {
        Elem(time_point when, std::function<void ()> what) : when(when), what(what) {}
        
        //Reversed order, as we want the first action, not the last
        bool operator< (const Elem& rhs) const { return this->when > rhs.when; }
        
        time_point when;
        std::function<void ()> what;
    };

    std::priority_queue<Elem> actions;
    std::mutex m;
    std::condition_variable c;
};

class UploadedFile
{
public:
    UploadedFile(const std::string& clientFileName,
                 const std::string& mimeType);
    
    std::string getClientFileName() const { return clientFileName; }
    
    std::string getServerFileName() const { return file.path(); }
    
    std::string getMimeType() const { return mimeType; }
    
    std::string getLinkKey() const { return linkKey; }
    
private:
    std::string clientFileName; ///< Name coming from client
    std::string mimeType;       ///< Mime type
    std::string linkKey;        ///< Key used to retrieve file
    Poco::TemporaryFile file;   ///< Tempfile is deleted by this class dtor
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
    
    std::map<std::string, std::shared_ptr<UploadedFile>> fm;
    std::mutex m;
};

#endif //FILE_H
