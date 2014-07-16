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

#ifndef APPLICATION_SERVER_H
#define APPLICATION_SERVER_H

#include <map>
#include <string>
#include <functional>
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerRequest.h"

/**
 * This is the base class of all page factories. There is a factory for each
 * page in the application server, using templates to specify which page,
 * see class PageFactory
 */
class PageFactoryBase
{
public:
    /**
     * Allocate on the heap a new HTTPRequestHandler to handle the required page
     */
    virtual Poco::Net::HTTPRequestHandler *generate()=0;
    
    /**
     * Destructor
     */
    virtual ~PageFactoryBase();
};

/**
 * This is the factory for a specific page
 * \param T the class, that must be a subclass of Poco::Net::HTTPRequestHandler
 * that serves the page
 */
template<typename T>
class PageFactory : public PageFactoryBase
{
public:
    /**
     * Allocate on the heap a new HTTPRequestHandler to handle the required page
     */
    virtual Poco::Net::HTTPRequestHandler *generate() { return new T; }
};

/**
 * This class is called when an HTTP request is done to serve the correct page.
 * It dispatches the request to the correct factory based on the page uri,
 * if a suitable one has been registered, or returns nullptr
 */
class ApplicationServer
{
public:
    /**
     * \return a pointer to this class (singleton)
     */
    static ApplicationServer *instance();
    
    /**
     * \param request HTTP request
     * \return a page, or nullptr if for the request uri no page was registered
     */
    Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request);

    /**
     * Register a page
     * \param name the associated page uri
     * \param factory factory to generate classes to serve the pages
     */
    void registerPage(const std::string& name, PageFactoryBase *factory);
    
private:
    /**
     * Costructor
     */
    ApplicationServer();
    ApplicationServer(const ApplicationServer&);
    ApplicationServer& operator= (const ApplicationServer&);
    
    /**
     * Destructor
     */
    virtual ~ApplicationServer();
    
    /// Known uri and page factories are stored here
    std::map<std::string,PageFactoryBase *> pages;
};

/**
 * Adapter class because HTTPServer wants the ownership of the pointer
 */
class RequestFactory : public Poco::Net::HTTPRequestHandlerFactory
{
public:
    /**
     * Constructor
     * \param srv the application server
     */
    RequestFactory(ApplicationServer *srv) : srv(srv) {}
    
    /**
     * \override HTTPRequestHandlerFactory::createRequestHandler()
     * \param request HTTP request
     * \return a page, or nullptr if for the request uri no page was registered
     */
    Poco::Net::HTTPRequestHandler* createRequestHandler(const Poco::Net::HTTPServerRequest& request)
    {
        return srv->createRequestHandler(request);
    }
    
private:
    ApplicationServer *srv;
};

/**
 * Decalre a global instance of this class in each .cpsp page to register it
 * with the application server. You can use the REGISTER_PAGE macro to help with
 * that
 */
template<typename T>
class RegisterPageAtStartup
{
public:
    /**
     * Constructor
     * \param name uri of the page, used to register it with the apllication
     * server
     */
    RegisterPageAtStartup(const std::string& name)
    {
        ApplicationServer::instance()->registerPage(name,new PageFactory<T>);
    }
};

/**
 * Use this macro in each .cpsp file to register the page to the application
 * server
 */
#define REGISTER_PAGE(x) static RegisterPageAtStartup<x> page(#x)

/**
 * Decalre a global instance of this class to execute the application model
 * initialization at startup without altering main(). You can use the
 * REGISTER_PAGE macro to help with that
 */
class RegisterFunctionAtStartup
{
public:
    /**
     * Constructor
     * \param a function that will called before main()
     */
    RegisterFunctionAtStartup(std::function<void ()> func) { func(); }
};

/**
 * Use this macro to register a function to initialize the application model
 */
#define REGISTER_FUNCTION(x) static RegisterFunctionAtStartup startupfunc(x)

#endif //APPLICATION_SERVER_H
