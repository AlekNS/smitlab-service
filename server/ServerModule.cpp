
#include <common/stdafx.h>


#include "ServerModule.h"


using namespace Poco;


//============================================================================//
ServerModule::ServerModule() 
{
}

//============================================================================//
const char* ServerModule::name() const 
{
    return "ServerModule";
}

//============================================================================//
ServerModule::~ServerModule() 
{
}

//============================================================================//
void ServerModule::initialize(Application& self) 
{
    poco_information(Logger::get(name()), "initialization");

    _server.start();
}

//============================================================================//
void ServerModule::uninitialize() 
{
    poco_information(Logger::get(name()), "shutting down");

    _server.stop();
}

