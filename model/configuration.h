
#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <vector>
#include <string>

class Configuration
{
public:
    static Configuration& instance();
    
    std::vector<std::string> allowedMimes;
    unsigned int maxFileSize;
    unsigned int maxFiles;
    unsigned int keepTime;
private:
    Configuration();
    Configuration(const Configuration&);
    Configuration& operator= (const Configuration&);
};

#endif //CONFIGURATION_H
