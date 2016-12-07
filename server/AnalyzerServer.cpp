
#include <common/stdafx.h>


#include <Poco/ThreadPool.h>
#include <Poco/Util/Application.h>
#include <Poco/Net/SSLManager.h>


#include "AnalyzerServer.h"
#include "AnalyzerRequests.h"
#include "../analyzer/AnalyzerModule.h"
#include "../service/SmitlabService.h"


using namespace std;


using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;


//============================================================================//
AnalyzerServer::AnalyzerServer()
{

}

//============================================================================//
AnalyzerServer::~AnalyzerServer()
{

}

//============================================================================//
void AnalyzerServer::fillRequestFactories(ServerRequestHandlerFactory *requestFactory)
{
    // requests for service
    requestFactory->registerServiceFactory<RequestAnalyzerList>("analyzerslist");

    // requests for analyzer
    requestFactory->registerAnalyzerFactory<RequestAnalyzerQueryPlaces>("queryplaces");
    // sample
    requestFactory->registerAnalyzerFactory<RequestAnalyzerPutSample>("putsample");
    requestFactory->registerAnalyzerFactory<RequestAnalyzerMoveSample>("movesample");
    requestFactory->registerAnalyzerFactory<RequestAnalyzerRemoveSample>("removesample");
    // groups
    requestFactory->registerAnalyzerFactory<RequestAnalyzerMoveResultGroup>("movegroup");
    requestFactory->registerAnalyzerFactory<RequestAnalyzerRemoveResultGroup>("removegroup");
    // params
    requestFactory->registerAnalyzerFactory<RequestAnalyzerQueryPlaceParameters>("queryplaceparameters");
    requestFactory->registerAnalyzerFactory<RequestAnalyzerSetPlaceParameter>("setplaceparameter");
    requestFactory->registerAnalyzerFactory<RequestAnalyzerRemovePlaceParameter>("removeplaceparameter");
    // config
    requestFactory->registerAnalyzerFactory<RequestAnalyzerSetConfig>("setconfig");
    // batch
    requestFactory->registerAnalyzerFactory<RequestAnalyzerBatch>("batch");
    
    // custom methods
    requestFactory->registerAnalyzerFactory<RequestAnalyzerInvokeMethod>("invoke");
}

//============================================================================//
void AnalyzerServer::startNotSafe()
{
    AbstractConfiguration &config = Application::instance().config();

    unsigned short port  = min(max(config.getInt("analyzers_host.port", 22222), 1), 65535);
    int maxQueued        = min(max(config.getInt("analyzers_host.max_requests", 50), 1), 100);
    int maxThreads       = min(max(config.getInt("analyzers_host.max_threads", 25), 1), 100);

    ThreadPool::defaultPool().addCapacity(maxThreads);

    HTTPServerParams* pParams = new HTTPServerParams;

    pParams->setSoftwareVersion("SmitlabService/" SMITLAB_SERVICE_VERSION_STRING);
    pParams->setMaxQueued(maxQueued);
    pParams->setMaxKeepAliveRequests(maxThreads);
    pParams->setMaxThreads(maxThreads);

    SocketAddress address(
        config.getString("analyzers_host.interface", "127.0.0.1"),
        port
    );

    _socketNotSafe = new ServerSocket(address);

    ServerRequestHandlerFactory *requestFactory = new ServerRequestHandlerFactory();

    fillRequestFactories(requestFactory);

    _serverNotSafe = new HTTPServer(
        requestFactory,
        *_socketNotSafe,
        pParams
    );

    _serverNotSafe->start();
}

//============================================================================//
void AnalyzerServer::startSecure()
{
    AbstractConfiguration &config = Application::instance().config();
    
    //Context::Ptr pDefaultServerContext = SSLManager::instance().defaultServerContext();
    //Context::Ptr pDefaultClientContext = SSLManager::instance().defaultClientContext();
     
    _secureContext = new Context(
        Context::SERVER_USE,
        config.getString("analyzers_secure_host.key"),
        config.getString("analyzers_secure_host.crt"),
        config.getString("analyzers_secure_host.ca"),
        Context::VERIFY_NONE,
        4,
        false
    );

//    _secureContext->enableSessionCache(true, "TestSuite");
    _secureContext->setSessionTimeout(10);
//    _secureContext->setSessionCacheSize(1000);
    _secureContext->disableStatelessSessionResumption();

    unsigned short port  = min(max(config.getInt("analyzers_secure_host.port", 22223), 1), 65535);
    int maxQueued        = min(max(config.getInt("analyzers_secure_host.max_requests", 50), 1), 100);
    int maxThreads       = min(max(config.getInt("analyzers_secure_host.max_threads", 25), 1), 100);

    ThreadPool::defaultPool().addCapacity(maxThreads);

    HTTPServerParams* pParams = new HTTPServerParams;

    pParams->setSoftwareVersion("SmitlabService/" SMITLAB_SERVICE_VERSION_STRING);
    pParams->setMaxQueued(maxQueued);
    pParams->setMaxKeepAliveRequests(maxThreads);
    pParams->setMaxThreads(maxThreads);

    SocketAddress address(
        config.getString("analyzers_secure_host.interface", "127.0.0.1"),
        port
    );

    _socketSecure = new SecureServerSocket(address, 64, _secureContext);

    ServerRequestHandlerFactory *requestFactory = new ServerRequestHandlerFactory();

    fillRequestFactories(requestFactory);

    _serverSecure = new HTTPServer(
        requestFactory,
        *_socketSecure,
        pParams
    );

    _serverSecure->start();
}


//============================================================================//
void AnalyzerServer::stopNotSafe() 
{
     _socketNotSafe->close();
     delete _socketNotSafe;
    
    _serverNotSafe->stop();
    delete _serverNotSafe;
}


//============================================================================//
void AnalyzerServer::stopSecure() 
{
    _secureContext->flushSessionCache();
    // _secureContext->release();

     _socketSecure->close();
     delete _socketSecure;

    _serverSecure->stop();
    delete _serverSecure;
}


//============================================================================//
void AnalyzerServer::start()
{
    AbstractConfiguration &config = Application::instance().config();

    AbstractConfiguration::Keys   configKeys;

    config.keys("analyzers_host", configKeys);

    if(configKeys.size() > 0 && !config.getString("analyzers_host.disabled", "enabled").compare("enabled"))
        startNotSafe();

    configKeys.clear();
    config.keys("analyzers_secure_host", configKeys);

    if(configKeys.size() > 0 && !config.getString("analyzers_secure_host.disabled", "enabled").compare("enabled"))
        startSecure();
}

//============================================================================//
void AnalyzerServer::stop()
{
    AbstractConfiguration &config = Application::instance().config();

    AbstractConfiguration::Keys   configKeys;
    
    config.keys("analyzers_host", configKeys);
    
    if(configKeys.size() > 0 && !config.getString("analyzers_host.disabled", "enabled").compare("enabled"))
        stopNotSafe();

    config.keys("analyzers_secure_host", configKeys);
    
    if(configKeys.size() > 0 && !config.getString("analyzers_secure_host.disabled", "enabled").compare("enabled"))
        stopSecure();
}

