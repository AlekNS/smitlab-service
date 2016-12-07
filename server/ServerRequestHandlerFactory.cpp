
#include <common/stdafx.h>


#include <Poco/URI.h>
#include <Poco/RegularExpression.h>
#include <Poco/Net/HTTPServerRequest.h>


#include "ServerRequestHandlerFactory.h"
#include "AnalyzerRequests.h"


using namespace std;


using namespace Poco;
using namespace Poco::Net;


//============================================================================//
ServerRequestHandlerFactory::ServerRequestHandlerFactory() :
    _analyzerRequestFabric(),
    _serviceRequestFabric()
{

}

//============================================================================//
ServerRequestHandlerFactory::~ServerRequestHandlerFactory()
{
}

//============================================================================//
HTTPRequestHandler* ServerRequestHandlerFactory::createRequestHandler(const HTTPServerRequest& request)
{
    try {

        URI uri(request.getURI());
        vector<string> pathSegment;

        uri.getPathSegments(pathSegment);
        string serviceMethod = pathSegment.size() > 0 ? pathSegment[0] : request.getURI();

        // call service method
        if(_serviceRequestFabric.isClass(serviceMethod)) {
            return _serviceRequestFabric.create(serviceMethod);
        }

        // call analyzers method
        if (pathSegment.size() == 3 && pathSegment[0] == "analyzers") {
            SharedPtr<Analyzer> analyzer;

            if(AnalyzerDispatcher::instance().getAnalyzer(analyzer, pathSegment[1]) &&
                _analyzerRequestFabric.isClass(pathSegment[2])) {
                return _analyzerRequestFabric.create(pathSegment[2], analyzer);
            }
        }

    }
    catch(Exception &e) {
        poco_error_f1(Logger::get("ServerRequest"), "exception [%s]", e.displayText());
    }
    catch(exception &e) {
        poco_error_f1(Logger::get("ServerRequest"), "exception [%s]", string(e.what()));
    }

    return NULL;
}
