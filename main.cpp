/***************************************************************************
 *   Copyright (C) 2014 by Terraneo Federico and Andrea Bontempi           *
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

#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "application_server.h"

#include <iostream>

using namespace Poco::Net;
using namespace Poco::Util;

class ServiceHandler : public Poco::Util::ServerApplication
{
protected:
    void defineOptions(OptionSet& options)
    {
        ServerApplication::defineOptions(options);
        options.addOption(
            Option("help", "h", "display help")
            .required(false)
            .repeatable(false));
        options.addOption(
            Option("port","p", "Listening port (default 9980)")
            .required(false)
            .repeatable(true)
            .argument("<port>"));
    }

    void handleOption(const std::string& name, const std::string& value)
    {
        ServerApplication::handleOption(name,value);
        if(name=="help") helpRequested=true;
        if(name=="port") listenPort=stoi(value);
    }

    void displayHelp()
    {
        HelpFormatter helpFormatter(options());
        helpFormatter.setCommand(commandName());
        helpFormatter.setUsage("[options]");
        helpFormatter.setHeader("C++ server pages application server.");
        helpFormatter.format(std::cout);
    }

    int main(const std::vector<std::string>& args)
    {
        if(helpRequested)
        {
            displayHelp();
            return Application::EXIT_OK;
        }
        ServerSocket sockSrv(listenPort);
        HTTPServer httpSrv(new RequestFactory(ApplicationServer::instance()),
                           sockSrv,new HTTPServerParams);
        httpSrv.start();
        waitForTerminationRequest();
        httpSrv.stop();
        return Application::EXIT_OK;
    }

private:
    bool helpRequested=false;
    int listenPort=9980;
};

int main(int argc, char* argv[])
{
    ServiceHandler sh;    
    return sh.run(argc,argv);
}
