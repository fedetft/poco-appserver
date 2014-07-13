
#include <iostream>
#include "application_server.h"
#include "file.h"

using namespace std;

REGISTER_FUNCTION([](){
    cout<<"Called"<<endl;
});

//
// class FileMap
//

FileMap& FileMap::instance()
{
    static FileMap singleton;
    return singleton;
}

FileMap::FileMap() {}
