
#ifndef FILE_H
#define FILE_H

#include <map>
#include <string>
#include <utility>
#include <mutex>
#include "Poco/TemporaryFile.h"
#include "Poco/MD5Engine.h"

class FileMap
{
public:
    static FileMap& instance();
    
    std::pair<std::string,std::string> add(const std::string& name)
    {
        std::lock_guard<std::mutex> l(m);
        Poco::TemporaryFile temp;
        temp.keepUntilExit();
        std::string tempname=temp.path();
        Poco::MD5Engine md5;
        md5.update(tempname);
        std::string key=Poco::DigestEngine::digestToHex(md5.digest());
        fm[key]=make_pair(tempname,name);
        return make_pair(key,tempname);
    }
    
    std::pair<std::string,std::string> get(const std::string& param)
    {
        std::lock_guard<std::mutex> l(m);
        return fm[param];
    }
        
private:
    FileMap();
    
    std::map<std::string, std::pair<std::string,std::string>> fm;
    std::mutex m;
};

#endif //FILE_H
