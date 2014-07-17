
#include "configuration.h"
#include "configfile.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "application_server.h"

using namespace std;

REGISTER_FUNCTION([]()
{
    //Force loading the configuration file at startup
    Configuration::instance();
});

//
// class Configuration
//

Configuration& Configuration::instance()
{
    static Configuration singleton;
    return singleton;
}

Configuration::Configuration()
{
    ConfigFile cfg("config.ini");
    string mimes=cfg.getString("allowedMimes");
    for(size_t a=0;;)
    {
        size_t b=mimes.find_first_of(':',a);
        string mime;
        if(b==string::npos) mime=mimes.substr(a);
        else mime=mimes.substr(a,b-a);
        if(!mime.empty()) allowedMimes.push_back(mime);
        if(b==string::npos) break;
        a=b+1;
    }
    stringstream ss(cfg.getString("maxFileSize"));
    ss.exceptions(ios::failbit);
    ss>>maxFileSize;
    char c;
    ss>>c;
    switch(c)
    {
        case 'K':
            maxFileSize*=1024;
            break;
        case 'M':
            maxFileSize*=1024*1024;
            break;
        case 'G':
            maxFileSize*=1024*1024*1024;
            break;
        default:
            throw runtime_error("Bad maxFileSize in config file");
    }
    maxFiles=stoul(cfg.getString("maxFiles"));
    keepTime=stoul(cfg.getString("keepTime"));
    authKey=cfg.getString("authKey");
}
