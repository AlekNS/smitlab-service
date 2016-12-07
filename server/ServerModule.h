#ifndef SERVER_MODULE_H
#define SERVER_MODULE_H


#include <Poco/Net/ServerSocket.h>
#include <Poco/Util/Subsystem.h>


#include "AnalyzerServer.h"


using Poco::Util::Application;
using Poco::Util::Subsystem;


class ServerModule : public Subsystem 
{
public:

    ServerModule();
    const char* name() const;

protected:

    void initialize(Application& self);
    void uninitialize();

    virtual ~ServerModule();

    AnalyzerServer              _server;

};


#endif
