
#ifndef SERVER_REQUEST_HANDLER_FACTORY_H
#define SERVER_REQUEST_HANDLER_FACTORY_H


#include <map>
#include <string>


#include <Poco/Net/HTTPRequestHandlerFactory.h>


#include "../analyzer/AnalyzerDispatcher.h"


using std::map;
using std::pair;
using std::string;


using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPServerResponse;
using Poco::Net::HTTPServerRequest;


class RequestFactory;


//============================================================================//
class ServerRequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:

    ServerRequestHandlerFactory();
    virtual ~ServerRequestHandlerFactory();

    HTTPRequestHandler* createRequestHandler(const HTTPServerRequest& request);

    template<class ClassType>
    void registerServiceFactory(const string& method) {
        _serviceRequestFabric.registerClass<ClassType>(method);
    }
    template<class ClassType>
    void registerAnalyzerFactory(const string& method) {
        _analyzerRequestFabric.registerClass<ClassType>(method);
    }

protected:

    common::AbstractFabric<HTTPRequestHandler>                            _serviceRequestFabric;
    common::AbstractFabric<HTTPRequestHandler, SharedPtr<Analyzer> >      _analyzerRequestFabric;

};


#endif
