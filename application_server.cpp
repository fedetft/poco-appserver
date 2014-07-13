/***************************************************************************
 *   Copyright (C) 2014 by Terraneo Federico                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   As a special exception, if other files instantiate templates or use   *
 *   macros or inline functions from this file, or you compile this file   *
 *   and link it with other works to produce a work based on this file,    *
 *   this file does not by itself cause the resulting work to be covered   *
 *   by the GNU General Public License. However the source code for this   *
 *   file must still be made available in accordance with the GNU General  *
 *   Public License. This exception does not invalidate any other reasons  *
 *   why a work based on this file might be covered by the GNU General     *
 *   Public License.                                                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#include <iostream>
#include <stdexcept>
#include "application_server.h"

using namespace std;
using namespace Poco::Net;

//
// class PageFactoryBase
//

PageFactoryBase::~PageFactoryBase() {}

//
// class ApplicationServer
//

ApplicationServer* ApplicationServer::instance()
{
    static ApplicationServer singleton;
    return &singleton;
}

HTTPRequestHandler* ApplicationServer::createRequestHandler(const HTTPServerRequest& request)
{
    string uri=request.getURI();
    if(uri.empty() || uri.at(0)!='/') return nullptr;
    uri.erase(0,1); //Strip beginning /
    size_t param=uri.find_first_of('?');
    if(param!=string::npos) uri=uri.substr(0,param);
    if(uri.empty()) uri="Index";
    auto it=pages.find(uri);
    if(it==pages.end()) return nullptr;
    return it->second->generate();
}

void ApplicationServer::registerPage(const std::string& name, PageFactoryBase *factory)
{
    if(pages.insert(make_pair(name,factory)).second==false)
            throw logic_error(string("attempting to register twice ")+name);
}

ApplicationServer::ApplicationServer() {}

ApplicationServer::~ApplicationServer()
{
    for(auto p : pages) delete p.second;
}
