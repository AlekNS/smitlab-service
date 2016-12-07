
#ifndef ANALYZER_REQUESTS_H
#define ANALYZER_REQUESTS_H


#include <vector>

#include <Poco/SharedPtr.h>
#include <Poco/Net/HTTPRequestHandler.h>


#include "../analyzer/Analyzer.h"
#include "../analyzer/AnalyzerStorage.h"
#include "InterpreterHolder.h"


using std::vector;


using Poco::SharedPtr;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;


namespace common { class DataEmitter; }
class Analyzer;


//============================================================================//
class BaseRequestServerHandler : public HTTPRequestHandler
{
public:

    BaseRequestServerHandler();
    virtual ~BaseRequestServerHandler();

protected:

    SharedPtr<InterpreterHolder>        _interpreter;

};

//============================================================================//
class RequestAnalyzerList : public BaseRequestServerHandler
{
public:

    RequestAnalyzerList();
    virtual ~RequestAnalyzerList();

    virtual void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response);
};




//============================================================================//
class RequestAnalyzerQueryPlaces : public BaseRequestServerHandler
{
public:

    RequestAnalyzerQueryPlaces(SharedPtr<Analyzer> analyzer);
    virtual ~RequestAnalyzerQueryPlaces();

    virtual void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response);

protected:
    void emitGroups(common::DataEmitter *emitter, vector<ObjectAnalyzerResultGroup> &groups, bool skipEmptyResult = false);

    SharedPtr<Analyzer>          _analyzer;

};

//============================================================================//
class RequestAnalyzerBatch : public BaseRequestServerHandler
{
public:

    RequestAnalyzerBatch(SharedPtr<Analyzer> analyzer);
    virtual ~RequestAnalyzerBatch();

    virtual void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response);

protected:

    SharedPtr<Analyzer>          _analyzer;

};

//============================================================================//
class RequestAnalyzerPutSample : public BaseRequestServerHandler
{
public:

    RequestAnalyzerPutSample(SharedPtr<Analyzer> analyzer);
    virtual ~RequestAnalyzerPutSample();

    virtual void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response);

protected:

    SharedPtr<Analyzer>          _analyzer;

};


//============================================================================//
class RequestAnalyzerMoveSample : public BaseRequestServerHandler
{
public:

    RequestAnalyzerMoveSample(SharedPtr<Analyzer> analyzer);
    virtual ~RequestAnalyzerMoveSample();

    virtual void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response);

protected:

    SharedPtr<Analyzer>          _analyzer;

};


//============================================================================//
class RequestAnalyzerRemoveSample : public BaseRequestServerHandler
{
public:

    RequestAnalyzerRemoveSample(SharedPtr<Analyzer> analyzer);
    virtual ~RequestAnalyzerRemoveSample();

    virtual void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response);

protected:

    SharedPtr<Analyzer>          _analyzer;

};


//============================================================================//
class RequestAnalyzerMoveResultGroup : public BaseRequestServerHandler
{
public:

    RequestAnalyzerMoveResultGroup(SharedPtr<Analyzer> analyzer);
    virtual ~RequestAnalyzerMoveResultGroup();

    virtual void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response);

protected:

    SharedPtr<Analyzer>          _analyzer;

};


//============================================================================//
class RequestAnalyzerRemoveResultGroup : public BaseRequestServerHandler
{
public:

    RequestAnalyzerRemoveResultGroup(SharedPtr<Analyzer> analyzer);
    virtual ~RequestAnalyzerRemoveResultGroup();

    virtual void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response);

protected:

    SharedPtr<Analyzer>          _analyzer;

};


//============================================================================//
class RequestAnalyzerQueryPlaceParameters : public BaseRequestServerHandler
{
public:

    RequestAnalyzerQueryPlaceParameters(SharedPtr<Analyzer> analyzer);
    virtual ~RequestAnalyzerQueryPlaceParameters();

    virtual void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response);

    SharedPtr<Analyzer>          _analyzer;

};

//============================================================================//
class RequestAnalyzerSetPlaceParameter : public BaseRequestServerHandler
{
public:

    RequestAnalyzerSetPlaceParameter(SharedPtr<Analyzer> analyzer);
    virtual ~RequestAnalyzerSetPlaceParameter();

    virtual void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response);

protected:

    SharedPtr<Analyzer>          _analyzer;

};

//============================================================================//
class RequestAnalyzerRemovePlaceParameter : public BaseRequestServerHandler
{
public:

    RequestAnalyzerRemovePlaceParameter(SharedPtr<Analyzer> analyzer);
    virtual ~RequestAnalyzerRemovePlaceParameter();

    virtual void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response);

protected:

    SharedPtr<Analyzer>          _analyzer;

};

//============================================================================//
class RequestAnalyzerInvokeMethod : public BaseRequestServerHandler
{
public:

    RequestAnalyzerInvokeMethod(SharedPtr<Analyzer> analyzer);
    virtual ~RequestAnalyzerInvokeMethod();

    virtual void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response);

protected:

    SharedPtr<Analyzer>          _analyzer;

};

//============================================================================//
class RequestAnalyzerSetConfig : public BaseRequestServerHandler
{
public:

    RequestAnalyzerSetConfig(SharedPtr<Analyzer> analyzer);
    virtual ~RequestAnalyzerSetConfig();

    virtual void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response);

protected:

    SharedPtr<Analyzer>          _analyzer;

};


#endif
