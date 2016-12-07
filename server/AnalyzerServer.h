
#ifndef ANALYZER_SERVER_H
#define ANALYZER_SERVER_H


#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/Context.h>
#include <Poco/Net/SecureServerSocket.h>


#include "ServerRequestHandlerFactory.h"


using Poco::Net::ServerSocket;
using Poco::Net::SecureServerSocket;
using Poco::Net::Context;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServerParams;


//============================================================================//
class AnalyzerServer
{
public:

    AnalyzerServer();
    ~AnalyzerServer();

    void start();
    void stop();
    
protected:

    void startSecure();
    void startNotSafe();

    void stopSecure();
    void stopNotSafe();

    void fillRequestFactories(ServerRequestHandlerFactory *factory);

    HTTPServer              *_serverNotSafe;
    ServerSocket            *_socketNotSafe;

    Context::Ptr            _secureContext;
    HTTPServer              *_serverSecure;
    SecureServerSocket      *_socketSecure;

};


#endif
