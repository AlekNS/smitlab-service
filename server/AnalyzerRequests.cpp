
#include <common/stdafx.h>


#include <Poco/URI.h>
#include <Poco/Format.h>
#include <Poco/NumberParser.h>
#include <Poco/RegularExpression.h>
#include <Poco/StringTokenizer.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>


#include "AnalyzerRequests.h"
#include "../analyzer/Analyzer.h"
#include "../analyzer/AnalyzerStorage.h"
#include "../analyzer/AnalyzerDispatcher.h"
#include "../common/JsonEmitter.h"
#include "llex.h"


using namespace std;


using namespace Poco;
using namespace Poco::Net;


using namespace common;


//============================================================================//
BaseRequestServerHandler::BaseRequestServerHandler() : _interpreter(new InterpreterHolder)
{
}

//============================================================================//
BaseRequestServerHandler::~BaseRequestServerHandler() { }




//
// Hard code JSONEmitter, JSONP use for crossdomain quering.
//

//============================================================================//
RequestAnalyzerList::RequestAnalyzerList() { }

//============================================================================//
RequestAnalyzerList::~RequestAnalyzerList() { }


class RequestAnalyzerJSONPCallback
{
public:
    static void begin(HTMLForm &params, std::ostream& ostr);
    static void end(HTMLForm &params, std::ostream& ostr);
};


void RequestAnalyzerJSONPCallback::begin(HTMLForm &params, std::ostream& ostr)
{
    if(params.has("callback")) {
        ostr << params["callback"];
        ostr << "(";
    }
}

void RequestAnalyzerJSONPCallback::end(HTMLForm &params, std::ostream& ostr)
{
    if(params.has("callback")) {
        ostr << ")";
    }
}


//============================================================================//
void RequestAnalyzerList::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
    try {
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json; charset=utf-8");
        response.set("Connection", "close");
        response.set("Pragma", "no-cache");

        std::ostream& ostr = response.send();

        HTMLForm params(request);
        RequestAnalyzerJSONPCallback::begin(params, ostr);

        SharedPtr<DataEmitter> emitter = new JsonDataEmitter(ostr);

        map<string, SharedPtr<Analyzer> >::iterator
            analyzerIter    = AnalyzerDispatcher::instance().getAnalyzers().begin(),
            analyzerIterEnd = AnalyzerDispatcher::instance().getAnalyzers().end();

        emitter->list();
            while(analyzerIter != analyzerIterEnd) {

                emitter->dict();
                    emitter->key("type");
                    emitter->item(analyzerIter->second->getType());

                    emitter->key("title");
                    emitter->item(analyzerIter->second->getTitle());

                    emitter->key("code");
                    emitter->item(analyzerIter->second->getCode());

                    emitter->key("methods");
                    emitter->list();

                    vector<string>           methods;
                    vector<string>::iterator methodName;
                    analyzerIter->second->getCallbacks(methods);
                    for(methodName = methods.begin(); methodName != methods.end(); ++methodName) {
                        emitter->item(*methodName);
                    }

                    emitter->end();

                    emitter->key("configs");
                    emitter->dict();

                        vector<RowAnalyzerConfig> configs;
                        RowAnalyzerConfig::iter   confIter, confEnd;

                        analyzerIter->second->storage()->queryConfig(configs, "");

                        confIter = configs.begin();
                        confEnd  = configs.end();
                        while(confIter != confEnd) {
                            emitter->key(confIter->code);
                            emitter->item(confIter->value);
                            ++confIter;
                        }

                    emitter->end();

                    emitter->key("parameters");
                    emitter->list();

                        vector<RowAnalyzerParameter> parameters;
                        RowAnalyzerParameter::iter   paramIter, paramEnd;

                        analyzerIter->second->storage()->queryParameter(parameters, "");

                        paramIter = parameters.begin();
                        paramEnd  = parameters.end();
                        while(paramIter != paramEnd) {
                            emitter->list();

                            emitter->item(paramIter->research_code);
                            emitter->item(paramIter->test_code);
                            emitter->item(paramIter->test_parameter_code);
                            emitter->item(paramIter->type);

                            emitter->end();
                            ++paramIter;
                        }

                    emitter->end();

                    emitter->key("places");
                    emitter->list();

                        vector<RowAnalyzerPlace> places;
                        RowAnalyzerPlace::iter   placeIter, placeEnd;

                        analyzerIter->second->storage()->queryPlace(places, "");

                        placeIter = places.begin();
                        placeEnd  = places.end();
                        while(placeIter != placeEnd) {
                            emitter->dict();

                                emitter->key("code");
                                emitter->item(placeIter->code);

                                emitter->key("title");
                                emitter->item(placeIter->title);

                            emitter->end();
                            ++placeIter;
                        }

                    emitter->end();

                    ++analyzerIter;
                emitter->end();
            }
        emitter->end();

        RequestAnalyzerJSONPCallback::end(params, ostr);
    }
    catch(Exception &e) {
        poco_error_f1(Logger::get("RequestAnalyzerList"), "exception [%s]", e.displayText());
    }
    catch(exception &e) {
        poco_error_f1(Logger::get("RequestAnalyzerList"), "exception [%s]", string(e.what()));
    }
}


//============================================================================//
RequestAnalyzerQueryPlaces::RequestAnalyzerQueryPlaces(SharedPtr<Analyzer> analyzer) :
  _analyzer(analyzer) {
}

//============================================================================//
RequestAnalyzerQueryPlaces::~RequestAnalyzerQueryPlaces() {

}


//============================================================================//
void RequestAnalyzerQueryPlaces::emitGroups(DataEmitter *emitter, 
    vector<ObjectAnalyzerResultGroup> &groups, bool skipEmptyResult)
{
    ObjectAnalyzerResultGroup::iter gb, ge;
    ObjectAnalyzerResult::iter rb, re;

    gb = groups.begin(); ge = groups.end();

    emitter->list();

    while(gb != ge) {

        _interpreter->setResultGroup(*gb);

        emitter->dict();

            emitter->key("id");
            emitter->item(gb->id);

            emitter->key("type");
            emitter->item(gb->measure_type);

            emitter->key("stamp");
            emitter->item(gb->measure_stamp);

            emitter->key("status");
            emitter->item(gb->status);

            emitter->key("comment");
            emitter->item(gb->comment);

            rb = gb->results.begin(); re = gb->results.end();

            emitter->key("results");
            emitter->list();
            while(rb != re) {
                if(skipEmptyResult && !rb->val.size() && !rb->measure_stamp.size()) {
                    ++rb;
                    continue;
                }

                emitter->dict();

                emitter->key("test");
                emitter->item(rb->parameter.test_code);

                emitter->key("code");
                emitter->item(rb->parameter.test_parameter_code);

                emitter->key("stamp");
                emitter->item(rb->measure_stamp);

                emitter->key("units");
                try {
                    if(!rb->status.compare("ok") || !rb->status.compare("empty")) {
                        if(rb->parameter.expression.size() > 0) {
                            try {
                                _interpreter->setExpressionValue(*rb);
                                emitter->item(_interpreter->getUnit());
                            }
                            catch(InterpreterHolderException &e) {
                                poco_error_f2(Logger::get("RequestAnalyzerQueryPlaces"), "calculation exception [%s] for [%s]", 
                                    e.displayText(), rb->parameter.code);
                                emitter->item("??Ошибка!!");
                            }
                        }
                        else {
                            emitter->item(rb->units);
                        }
                    }
                    else {
                        emitter->item(rb->units);
                    }
                }
                catch(Exception &e) {
                    emitter->item(e.displayText());
                }
                catch(...) {
                    emitter->item("---");
                }

                emitter->key("val");
                try {
                    if(!rb->status.compare("ok") || !rb->status.compare("empty")) {
                        if(rb->parameter.expression.size() > 0) {
                            try {
                                _interpreter->setExpressionValue(*rb);
                                emitter->item(_interpreter->getValue());
                            }
                            catch(InterpreterHolderException &e) {
                                poco_error_f2(Logger::get("RequestAnalyzerQueryPlaces"), "calculation exception [%s] for [%s]", 
                                    e.displayText(), rb->parameter.code);
                                emitter->item("??Ошибка!!");
                            }
                        }
                        else {
                            emitter->item(rb->val);
                        }
                    }
                    else {
                        emitter->item(rb->val);
                    }
                }
                catch(Exception &e) {
                    emitter->item(e.displayText());
                }
                catch(...) {
                    emitter->item("---");
                }

                emitter->key("stat");
                emitter->item(rb->status);

                emitter->key("cmnt");
                emitter->item(rb->comment);

                emitter->end();

                ++rb;
            }
            emitter->end();

        emitter->end();

        ++gb;
    }

    emitter->end();
}

//============================================================================//
void RequestAnalyzerQueryPlaces::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{

    try {
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json; charset=utf-8");
        response.set("Connection", "close");
        response.set("Pragma", "no-cache");

        std::ostream& ostr = response.send();

        HTMLForm params(request);
        RequestAnalyzerJSONPCallback::begin(params, ostr);

        _analyzer->invokeCallback("_onQueryPlaces");

        SharedPtr<DataEmitter> emitter = new JsonDataEmitter(ostr);

        emitter->list();

    int passedCacheId = -1; // storage cacheid start at 1, after 2^32, it's impossible to repeat
    int storageCacheId = _analyzer->storage()->getCacheId();

        emitter->dict();
            emitter->key("cacheid");
            emitter->item(storageCacheId);

    if(params.has("cacheid") && NumberParser::tryParse(params["cacheid"], passedCacheId) &&
        storageCacheId == passedCacheId) {

            emitter->end();
            emitter->end();
            RequestAnalyzerJSONPCallback::end(params, ostr);
        return;
    }

        emitter->key("places");
        emitter->list();

        // @TODO: Refactor!
        vector<RowAnalyzerConfig> configs;
        bool skipEmptyResult = true;
        _analyzer->storage()->queryConfig(configs, "information.response.emptyResults");
        if(configs.size() && !configs[0].value.compare("true")) {
            skipEmptyResult = false;
        }

        configs.clear();

        _analyzer->storage()->queryConfig(configs, "");
        _interpreter->setConfig(configs);

        bool withResults = true, withParameters = true;

        vector<ObjectAnalyzerPlace> places;
        ObjectAnalyzerPlace::iter   pb, pe;
        ObjectAnalyzerSample::iter  sb, se;
        string                      placeCode;

        if(params.has("place"))
            placeCode = params["place"];

        _analyzer->storage()->queryPlaceDetailed(places, placeCode, withResults, withParameters);

        pb = places.begin(); pe = places.end();

        while(pb!=pe) {
            emitter->dict();

            emitter->key("place");
            emitter->item(pb->code);

            emitter->key("params");
            emitter->list();
            if(pb->parameters.size()) {
                RowAnalyzerPlaceParameter::iter ppb = pb->parameters.begin(),
                                                ppe = pb->parameters.end();

                while(ppb != ppe) {
                    emitter->dict();

                        emitter->key("code");
                        emitter->item(ppb->code);

                        emitter->key("value");
                        emitter->item(ppb->value);

                    emitter->end();
                    ++ppb;
                }
            }
            emitter->end();

            emitter->key("samples");
            emitter->list();

            sb = pb->samples.begin();
            se = pb->samples.end();

            while(sb != se) {
                emitter->dict();

                    emitter->key("uid");
                    emitter->item(sb->uid);

                    emitter->key("puid");
                    emitter->item(sb->patient_uid);

                    emitter->key("stamp");
                    emitter->item(sb->place_stamp);

                    emitter->key("status");
                    emitter->item(sb->status);

                    emitter->key("comment");
                    emitter->item(sb->comment);

                    emitter->key("groups");
                    emitGroups(emitter, sb->groups, skipEmptyResult);

                emitter->end();
                ++sb;
            }
            emitter->end();

            emitter->key("groups");
            emitGroups(emitter, pb->groups, skipEmptyResult);

            ++pb;
            emitter->end();
        }

        emitter->end();
        emitter->end();
        emitter->end(); // is need?

        RequestAnalyzerJSONPCallback::end(params, ostr);
    }
    catch(Exception &e) {
        poco_error_f1(Logger::get("RequestAnalyzerQueryPlaces"), "exception [%s]", e.displayText());
    }
    catch(exception &e) {
        poco_error_f1(Logger::get("RequestAnalyzerQueryPlaces"), "exception [%s]", string(e.what()));
    }
}




//============================================================================//
RequestAnalyzerBatch::RequestAnalyzerBatch(SharedPtr<Analyzer> analyzer) : _analyzer(analyzer) { }

//============================================================================//
RequestAnalyzerBatch::~RequestAnalyzerBatch() { }

//============================================================================//
void RequestAnalyzerBatch::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{

    try {
        //response.setChunkedTransferEncoding(true);
        response.setContentType("application/json; charset=utf-8");
        response.set("Connection", "close");
        response.set("Pragma", "no-cache");

        HTMLForm params(request);

        if(params.has("operation")) {

            string              status = "success";
            StringTokenizer     tokens(params["operation"], ",");

            if(tokens.count() == 0) {
                response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST, HTTPResponse::HTTP_REASON_BAD_REQUEST);
                response.setContentLength(0);
                response.send();
                return;
            }

            _analyzer->invokeCallback("_onBatch", params["operation"]);

            ObjectAnalyzerResultGroup::iter     rgb, rge;
            RowAnalyzerSample::iter             sb, se;

            vector<RowAnalyzerSample>           samples;
            vector<ObjectAnalyzerResultGroup>   resultGroups;

            _analyzer->storage()->querySample(samples, 0, "");

            sb = samples.begin(); se = samples.end();

            _analyzer->storage()->queryResultGroup(resultGroups, "", 0, "%");
            rgb = resultGroups.begin(); rge = resultGroups.end();

            if(!params["operation"].compare("removeAllResultsGroups")) {
                for(;rgb != rge; ++rgb) {
                    if(rgb->sample_uid.size())      continue;
                    _analyzer->storage()->removeResultGroup(rgb->id);
                }
            }
            else if(!params["operation"].compare("removeAllSamplesResultsGroups")) {
                for(;rgb != rge; ++rgb) {
                    if(!rgb->sample_uid.size())     continue;
                    _analyzer->storage()->removeResultGroup(rgb->id);
                }
            }
            else if(!params["operation"].compare("removeAllSamples")) {
                for(;sb != se; ++sb) {
                    _analyzer->storage()->removeSample(sb->uid);
                }
            }
            else {
                status = "unknown";
            }

            std::ostream& ostr = response.send();

            RequestAnalyzerJSONPCallback::begin(params, ostr);

            SharedPtr<DataEmitter> emitter = new JsonDataEmitter(ostr);

            emitter->list();
                emitter->dict();

                    emitter->key("operation");
                    emitter->item(params["operation"]);

                    emitter->key("status");
                    emitter->item(status);

                emitter->end();
            emitter->end();

            RequestAnalyzerJSONPCallback::end(params, ostr);
        }
        else {
            response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST, HTTPResponse::HTTP_REASON_BAD_REQUEST);
            response.setContentLength(0);
            response.send();
        }

    }
    catch(Exception &e) {
        poco_error_f1(Logger::get("RequestAnalyzerBatch"), "exception [%s]", e.displayText());
    }
    catch(exception &e) {
        poco_error_f1(Logger::get("RequestAnalyzerBatch"), "exception [%s]", string(e.what()));
    }

}




//============================================================================//
RequestAnalyzerPutSample::RequestAnalyzerPutSample(SharedPtr<Analyzer> analyzer) : _analyzer(analyzer) { }

//============================================================================//
RequestAnalyzerPutSample::~RequestAnalyzerPutSample() { }

//============================================================================//
void RequestAnalyzerPutSample::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{

    try {
        //response.setChunkedTransferEncoding(true);
        response.setContentType("application/json; charset=utf-8");
        response.set("Connection", "close");
        response.set("Pragma", "no-cache");

        HTMLForm params(request);

        if(params.has("sample")) {

            StringTokenizer tokens(params["sample"], ",");

            if(tokens.count() < 3) {
                response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST, HTTPResponse::HTTP_REASON_BAD_REQUEST);
                response.setContentLength(0);
                response.send();
                return;
            }

            int placeId = 0;

            vector<RowAnalyzerSample> samples;
            vector<RowAnalyzerPlace>  places;

            _analyzer->invokeCallback("_onPutSample", params["sample"]);

            _analyzer->storage()->queryPlace(places, tokens[2]);

            if(places.size() != 1) {
                response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST, HTTPResponse::HTTP_REASON_BAD_REQUEST);
                response.setContentLength(0);
                response.send();
                return;
            }

            placeId = places[0].id;

            _analyzer->storage()->querySample(samples, placeId, "");

            if(samples.size() != 0) {
                response.setStatusAndReason(HTTPResponse::HTTP_CONFLICT, HTTPResponse::HTTP_REASON_CONFLICT);
                response.setContentLength(0);
                response.send();
                return;
            }


            try {
                string patientSex = tokens.count() > 3 ? tokens[3] : "", 
                       patientBirthday = tokens.count() > 4 ? tokens[4] : "",
                       patientFullname = tokens.count() > 5 ? tokens[5] : "";
                
                if(patientSex.compare("M") && patientSex.compare("F") &&
                   patientSex.compare("U")) {
                    patientSex = "U";
                }
                
                // @TODO: validate birthday
                if(patientBirthday.size() != 8) {
                    patientBirthday = "";
                }
                
                if(patientFullname.size() > 32) {
                    patientFullname = patientFullname.substr(0, 32);
                }
                
                _analyzer->storage()->begin();
                _analyzer->storage()->addSample(tokens[0], tokens[1], patientSex, patientBirthday, patientFullname, placeId);
                _analyzer->storage()->commit();
            }
            catch(Exception &e) {
                _analyzer->storage()->rollback();
                response.setStatusAndReason(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, HTTPResponse::HTTP_REASON_INTERNAL_SERVER_ERROR);
                response.setContentLength(0);
                response.send();
                return;
            }


            std::ostream& ostr = response.send();

            RequestAnalyzerJSONPCallback::begin(params, ostr);

            SharedPtr<DataEmitter> emitter = new JsonDataEmitter(ostr);

            emitter->list();
                emitter->dict();
                    emitter->key("uid");
                    emitter->item(tokens[0]);
                    emitter->key("puid");
                    emitter->item(tokens[1]);
                    emitter->key("place");
                    emitter->item(tokens[2]);
                emitter->end();
            emitter->end();

            RequestAnalyzerJSONPCallback::end(params, ostr);
        }
        else {
            response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST, HTTPResponse::HTTP_REASON_BAD_REQUEST);
            response.setContentLength(0);
            response.send();
        }

    }
    catch(Exception &e) {
        poco_error_f1(Logger::get("RequestAnalyzerPutSample"), "exception [%s]", e.displayText());
    }
    catch(exception &e) {
        poco_error_f1(Logger::get("RequestAnalyzerPutSample"), "exception [%s]", string(e.what()));
    }

}




//============================================================================//
RequestAnalyzerMoveSample::RequestAnalyzerMoveSample(SharedPtr<Analyzer> analyzer) : _analyzer(analyzer) { }

//============================================================================//
RequestAnalyzerMoveSample::~RequestAnalyzerMoveSample() { }

//============================================================================//
void RequestAnalyzerMoveSample::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{

    try {
        //response.setChunkedTransferEncoding(true);
        response.setContentType("application/json; charset=utf-8");
        response.set("Connection", "close");
        response.set("Pragma", "no-cache");

        HTMLForm params(request);

        if(params.has("sample") && params.has("toplace")) {

            StringTokenizer tokens(params["sample"], ",");

            vector<RowAnalyzerPlace>    places;

            _analyzer->invokeCallback("_onMoveSample", params["sample"] + "," + params["toplace"]);

            try {
                _analyzer->storage()->begin();

                _analyzer->storage()->queryPlace(places, params["toplace"]);

                if(places.size() != 1) {
                    response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST, HTTPResponse::HTTP_REASON_BAD_REQUEST);
                    response.setContentLength(0);
                    response.send();
                    return;
                }

                _analyzer->storage()->moveSampleToPlace(params["sample"], places[0].id);
                _analyzer->storage()->commit();
            }
            catch(Exception &e) {
                _analyzer->storage()->rollback();
                response.setStatusAndReason(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, HTTPResponse::HTTP_REASON_INTERNAL_SERVER_ERROR);
                response.setContentLength(0);
                response.send();
                return;
            }


            std::ostream& ostr = response.send();

            RequestAnalyzerJSONPCallback::begin(params, ostr);

            SharedPtr<DataEmitter> emitter = new JsonDataEmitter(ostr);

            emitter->list();
                emitter->dict();
                    emitter->key("sample");
                    emitter->item(params["sample"]);
                emitter->end();
            emitter->end();

            RequestAnalyzerJSONPCallback::end(params, ostr);
        }
        else {
            response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST, HTTPResponse::HTTP_REASON_BAD_REQUEST);
            response.setContentLength(0);
            response.send();
        }
    }
    catch(Exception &e) {
        poco_error_f1(Logger::get("RequestAnalyzerMoveSample"), "exception [%s]", e.displayText());
    }
    catch(exception &e) {
        poco_error_f1(Logger::get("RequestAnalyzerMoveSample"), "exception [%s]", string(e.what()));
    }

}




//============================================================================//
RequestAnalyzerRemoveSample::RequestAnalyzerRemoveSample(SharedPtr<Analyzer> analyzer) : _analyzer(analyzer) { }

//============================================================================//
RequestAnalyzerRemoveSample::~RequestAnalyzerRemoveSample() { }

//============================================================================//
void RequestAnalyzerRemoveSample::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{

    try {
        //response.setChunkedTransferEncoding(true);
        response.setContentType("application/json; charset=utf-8");
        response.set("Connection", "close");
        response.set("Pragma", "no-cache");

        HTMLForm params(request);

        if(params.has("sample")) {

            vector<RowAnalyzerSample> samples;


            _analyzer->storage()->querySample(samples, 0, params["sample"]);

            if(samples.size() != 1) {
                response.setStatusAndReason(HTTPResponse::HTTP_NOT_FOUND, HTTPResponse::HTTP_REASON_NOT_FOUND);
                response.setContentLength(0);
                response.send();
                return;
            }

            _analyzer->invokeCallback("_onRemoveSample", params["sample"]);

            try {
                _analyzer->storage()->begin();
                _analyzer->storage()->removeSample(params["sample"]);
                _analyzer->storage()->commit();
            }
            catch(Exception &e) {
                _analyzer->storage()->rollback();
                response.setStatusAndReason(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, HTTPResponse::HTTP_REASON_INTERNAL_SERVER_ERROR);
                response.setContentLength(0);
                response.send();
                return;
            }


            std::ostream& ostr = response.send();
            RequestAnalyzerJSONPCallback::begin(params, ostr);

            SharedPtr<DataEmitter> emitter = new JsonDataEmitter(ostr);

            emitter->list();
                emitter->dict();
                    emitter->key("uid");
                    emitter->item(params["sample"]);
                emitter->end();
            emitter->end();

            RequestAnalyzerJSONPCallback::end(params, ostr);
        }
        else {
            response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST, HTTPResponse::HTTP_REASON_BAD_REQUEST);
            response.setContentLength(0);
            response.send();
        }

    }
    catch(Exception &e) {
        poco_error_f1(Logger::get("RequestAnalyzerRemoveSample"), "exception [%s]", e.displayText());
    }
    catch(exception &e) {
        poco_error_f1(Logger::get("RequestAnalyzerRemoveSample"), "exception [%s]", string(e.what()));
    }

}




//============================================================================//
RequestAnalyzerMoveResultGroup::RequestAnalyzerMoveResultGroup(SharedPtr<Analyzer> analyzer) : _analyzer(analyzer) { }

//============================================================================//
RequestAnalyzerMoveResultGroup::~RequestAnalyzerMoveResultGroup() { }

//============================================================================//
void RequestAnalyzerMoveResultGroup::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{

    try {
        //response.setChunkedTransferEncoding(true);
        response.setContentType("application/json; charset=utf-8");
        response.set("Connection", "close");
        response.set("Pragma", "no-cache");

        HTMLForm params(request);

        if(params.has("group") && params.has("toplace")) {

            unsigned groupId = 0, placeId = 0;

            if(!NumberParser::tryParseUnsigned(params["group"], groupId) || !params["toplace"].size()) {
                response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST, HTTPResponse::HTTP_REASON_BAD_REQUEST);
                response.setContentLength(0);
                response.send();
                return;
            }

            _analyzer->invokeCallback("_onMoveResultGroup", params["group"] + "," + params["toplace"]);

            vector<RowAnalyzerSample>   samples;
            vector<RowAnalyzerPlace>    places;
            string                      sampleUid;

            if(params.has("sample"))
                sampleUid = params["sample"];

            try {
                _analyzer->storage()->begin();

                _analyzer->storage()->queryPlace(places, params["toplace"]);
                if(places.size() != 1) {
                    poco_error_f1(Logger::get("RequestAnalyzerMoveResultGroup"), "place not found [%s]", params["toplace"]);
                    throw IOException();
                }

                if(sampleUid.size() > 0) {
                    _analyzer->storage()->querySample(samples, places[0].id, sampleUid);
                    if(samples.size() != 1) {
                        sampleUid = "";
                    }
                }
                _analyzer->storage()->moveResultGroupToPlace(groupId, places[0].id, sampleUid);

                _analyzer->storage()->commit();
            }
            catch(Exception &e) {
                _analyzer->storage()->rollback();
                response.setStatusAndReason(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, HTTPResponse::HTTP_REASON_INTERNAL_SERVER_ERROR);
                response.setContentLength(0);
                response.send();
                return;
            }

            std::ostream& ostr = response.send();
            RequestAnalyzerJSONPCallback::begin(params, ostr);

            SharedPtr<DataEmitter> emitter = new JsonDataEmitter(ostr);

            emitter->list();
                emitter->dict();
                    emitter->key("group");
                    emitter->item(params["group"]);
                emitter->end();
            emitter->end();

            RequestAnalyzerJSONPCallback::end(params, ostr);
        }
        else {
            response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST, HTTPResponse::HTTP_REASON_BAD_REQUEST);
            response.setContentLength(0);
            response.send();
        }
    }
    catch(Exception &e) {
        poco_error_f1(Logger::get("RequestAnalyzerMoveResultGroup"), "exception [%s]", e.displayText());
    }
    catch(exception &e) {
        poco_error_f1(Logger::get("RequestAnalyzerMoveResultGroup"), "exception [%s]", string(e.what()));
    }

}




//============================================================================//
RequestAnalyzerRemoveResultGroup::RequestAnalyzerRemoveResultGroup(SharedPtr<Analyzer> analyzer) : _analyzer(analyzer) { }

//============================================================================//
RequestAnalyzerRemoveResultGroup::~RequestAnalyzerRemoveResultGroup() { }

//============================================================================//
void RequestAnalyzerRemoveResultGroup::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{

    try {
        //response.setChunkedTransferEncoding(true);
        response.setContentType("application/json; charset=utf-8");
        response.set("Connection", "close");
        response.set("Pragma", "no-cache");

        HTMLForm params(request);

        if(params.has("group")) {

            unsigned groupId = 0;

            if(!NumberParser::tryParseUnsigned(params["group"], groupId)) {
                response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST, HTTPResponse::HTTP_REASON_BAD_REQUEST);
                response.setContentLength(0);
                response.send();
                return;
            }

            _analyzer->invokeCallback("_onRemoveResultGroup", params["group"]);

            try {
                _analyzer->storage()->begin();
                _analyzer->storage()->removeResultGroup(groupId);
                _analyzer->storage()->commit();
            }
            catch(Exception &e) {
                _analyzer->storage()->rollback();
                response.setStatusAndReason(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, HTTPResponse::HTTP_REASON_INTERNAL_SERVER_ERROR);
                response.setContentLength(0);
                response.send();
                return;
            }

            std::ostream& ostr = response.send();
            RequestAnalyzerJSONPCallback::begin(params, ostr);

            SharedPtr<DataEmitter> emitter = new JsonDataEmitter(ostr);

            emitter->list();
                emitter->dict();
                    emitter->key("group");
                    emitter->item(params["group"]);
                emitter->end();
            emitter->end();

            RequestAnalyzerJSONPCallback::end(params, ostr);

        }
        else {
            response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST, HTTPResponse::HTTP_REASON_BAD_REQUEST);
            response.setContentLength(0);
            response.send();
        }
    }
    catch(Exception &e) {
        poco_error_f1(Logger::get("RequestAnalyzerRemoveResultGroup"), "exception [%s]", e.displayText());
    }
    catch(exception &e) {
        poco_error_f1(Logger::get("RequestAnalyzerRemoveResultGroup"), "exception [%s]", string(e.what()));
    }

}











//============================================================================//
RequestAnalyzerQueryPlaceParameters::RequestAnalyzerQueryPlaceParameters(SharedPtr<Analyzer> analyzer) : _analyzer(analyzer) { }

//============================================================================//
RequestAnalyzerQueryPlaceParameters::~RequestAnalyzerQueryPlaceParameters() { }

//============================================================================//
void RequestAnalyzerQueryPlaceParameters::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{

    try {
        //response.setChunkedTransferEncoding(true);
        response.setContentType("application/json; charset=utf-8");
        response.set("Connection", "close");
        response.set("Pragma", "no-cache");

        HTMLForm params(request);

        // @TODO: Make places filter
//        if(params.has("places")) {

            _analyzer->invokeCallback("_onQueryPlaceParameters");

            vector<RowAnalyzerPlaceParameter>    placeParams;
            try {
                _analyzer->storage()->begin();
                _analyzer->storage()->queryPlaceParameter(placeParams, 0, "");
                _analyzer->storage()->commit();
            }
            catch(Exception &e) {
                _analyzer->storage()->rollback();
                response.setStatusAndReason(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, HTTPResponse::HTTP_REASON_INTERNAL_SERVER_ERROR);
                response.setContentLength(0);
                response.send();
                return;
            }

            std::ostream& ostr = response.send();
            RequestAnalyzerJSONPCallback::begin(params, ostr);

            SharedPtr<DataEmitter> emitter = new JsonDataEmitter(ostr);

            emitter->list();
                emitter->dict();
                    emitter->key("places");
                    emitter->dict();

                    if(placeParams.size()) {
                        string prevCode;
                        bool putPlaceCode = false;

                        for(RowAnalyzerPlaceParameter::iter placeParam = placeParams.begin();
                                placeParam != placeParams.end();
                                placeParam += 1) {

                            if(prevCode.compare(placeParam->place_code)) {
                                prevCode = placeParam->place_code;

                                if(putPlaceCode) {
                                    emitter->end();
                                }
                                emitter->item(prevCode);
                                emitter->dict();

                                putPlaceCode = true;
                            }

                            emitter->key("code"); emitter->item(placeParam->code);
                            emitter->key("value"); emitter->item(placeParam->value);
                        }

                        emitter->end();
                    }

                    emitter->end();
                emitter->end();
            emitter->end();

            RequestAnalyzerJSONPCallback::end(params, ostr);

//        }
//        else {
//            response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST, HTTPResponse::HTTP_REASON_BAD_REQUEST);
//            response.setContentLength(0);
//            response.send();
//        }
    }
    catch(Exception &e) {
        poco_error_f1(Logger::get("RequestAnalyzerQueryPlaceParameters"), "exception [%s]", e.displayText());
    }
    catch(exception &e) {
        poco_error_f1(Logger::get("RequestAnalyzerQueryPlaceParameters"), "exception [%s]", string(e.what()));
    }

}





//============================================================================//
RequestAnalyzerSetPlaceParameter::RequestAnalyzerSetPlaceParameter(SharedPtr<Analyzer> analyzer) : _analyzer(analyzer) { }

//============================================================================//
RequestAnalyzerSetPlaceParameter::~RequestAnalyzerSetPlaceParameter() { }

//============================================================================//
void RequestAnalyzerSetPlaceParameter::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{

    try {
        //response.setChunkedTransferEncoding(true);
        response.setContentType("application/json; charset=utf-8");
        response.set("Connection", "close");
        response.set("Pragma", "no-cache");

        HTMLForm params;
        params.setFieldLimit(2048);
        params.load(request);

        if(params.has("places")) {

            StringTokenizer placeCodes(params["places"], ",");

            StringTokenizer::Iterator placeCode;
            for(placeCode = placeCodes.begin(); placeCode != placeCodes.end(); placeCode += 1) {

                // check parameter items
                if(!params.has(*placeCode)) {
                    response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST, HTTPResponse::HTTP_REASON_BAD_REQUEST);
                    response.setContentLength(0);
                    response.send();
                    return;
                }

            }

            _analyzer->invokeCallback("_onSetPlaceParameter", params["places"]);

            int processedCount = 0;


            try {
                _analyzer->storage()->begin();

                // query places
                vector<RowAnalyzerPlace> places;
                map<string, int>         placesMap;
                map<string, int>::const_iterator placeMap;

                _analyzer->storage()->queryPlace(places, "");

                for(RowAnalyzerPlace::iter place = places.begin();
                        place != places.end(); place += 1) {
                    placesMap.insert(make_pair(place->code, place->id));
                }

                // iterate over codes
                for(placeCode = placeCodes.begin(); placeCode != placeCodes.end(); placeCode += 1) {

                    placeMap = placesMap.find(*placeCode);
                    if(placeMap == placesMap.end()) continue;

                    StringTokenizer placeParameters(params[*placeCode], ",", 
                        StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);

                    StringTokenizer::Iterator parameter;
                    for(parameter = placeParameters.begin(); parameter != placeParameters.end();
                            parameter += 1) {

                        StringTokenizer items(*parameter, "=");

                        if(items.count() > 2 || !items.count()) continue;

                        _analyzer->storage()->setPlaceParameter(placeMap->second, items[0], items.count() == 1 ? string("") : items[1]);
                        processedCount += 1;
                    }

                }

                _analyzer->storage()->commit();
            }
            catch(Exception &e) {
                _analyzer->storage()->rollback();
                response.setStatusAndReason(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, HTTPResponse::HTTP_REASON_INTERNAL_SERVER_ERROR);
                response.setContentLength(0);
                response.send();
                return;
            }

            std::ostream& ostr = response.send();
            RequestAnalyzerJSONPCallback::begin(params, ostr);

            SharedPtr<DataEmitter> emitter = new JsonDataEmitter(ostr);

            emitter->list();
                emitter->dict();
                    emitter->key("processed");
                    emitter->item(processedCount);
                emitter->end();
            emitter->end();

            RequestAnalyzerJSONPCallback::end(params, ostr);
        }
        else {
            response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST, HTTPResponse::HTTP_REASON_BAD_REQUEST);
            response.setContentLength(0);
            response.send();
        }
    }
    catch(Exception &e) {
        poco_error_f1(Logger::get("RequestAnalyzerSetPlaceParameter"), "exception [%s]", e.displayText());
    }
    catch(exception &e) {
        poco_error_f1(Logger::get("RequestAnalyzerSetPlaceParameter"), "exception [%s]", string(e.what()));
    }

}




//============================================================================//
RequestAnalyzerRemovePlaceParameter::RequestAnalyzerRemovePlaceParameter(SharedPtr<Analyzer> analyzer) : _analyzer(analyzer) { }

//============================================================================//
RequestAnalyzerRemovePlaceParameter::~RequestAnalyzerRemovePlaceParameter() { }

//============================================================================//
void RequestAnalyzerRemovePlaceParameter::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{

    try {
        //response.setChunkedTransferEncoding(true);
        response.setContentType("application/json; charset=utf-8");
        response.set("Connection", "close");
        response.set("Pragma", "no-cache");

        HTMLForm params;
        params.setFieldLimit(2048);
        params.load(request);

        if(params.has("places")) {

            StringTokenizer::Iterator placeCode;
            StringTokenizer placeCodes(params["places"], ",");

            if(!placeCodes.count()) {
                response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST, HTTPResponse::HTTP_REASON_BAD_REQUEST);
                response.setContentLength(0);
                response.send();
                return;
            }

            int processedCount = 0;

            _analyzer->invokeCallback("_onRemovePlaceParameter", params["places"]);

            try {
                _analyzer->storage()->begin();

                // query places
                vector<RowAnalyzerPlace> places;
                map<string, int>         placesMap;
                map<string, int>::const_iterator placeMap;

                _analyzer->storage()->queryPlace(places, "");

                for(RowAnalyzerPlace::iter place = places.begin();
                        place != places.end(); place += 1) {
                    placesMap.insert(make_pair(place->code, place->id));
                }

                // iterate over codes
                for(placeCode = placeCodes.begin(); placeCode != placeCodes.end(); placeCode += 1) {

                    placeMap = placesMap.find(*placeCode);
                    if(placeMap == placesMap.end()) continue;

                    if(!params.has(*placeCode)) {
                        _analyzer->storage()->removePlaceParameter(placeMap->second, "");
                        processedCount += 1;
                        continue;
                    }

                    StringTokenizer placeParameters(params[*placeCode], ",", 
                        StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);

                    StringTokenizer::Iterator parameter;
                    for(parameter = placeParameters.begin(); parameter != placeParameters.end();
                            parameter += 1) {

                        _analyzer->storage()->removePlaceParameter(placeMap->second, *parameter);
                        processedCount += 1;
                    }

                }

                _analyzer->storage()->commit();
            }
            catch(Exception &e) {
                _analyzer->storage()->rollback();
                response.setStatusAndReason(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, HTTPResponse::HTTP_REASON_INTERNAL_SERVER_ERROR);
                response.setContentLength(0);
                response.send();
                return;
            }

            std::ostream& ostr = response.send();
            RequestAnalyzerJSONPCallback::begin(params, ostr);

            SharedPtr<DataEmitter> emitter = new JsonDataEmitter(ostr);

            emitter->list();
                emitter->dict();
                    emitter->key("processed");
                    emitter->item(processedCount);
                emitter->end();
            emitter->end();

            RequestAnalyzerJSONPCallback::end(params, ostr);
        }
        else {
            response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST, HTTPResponse::HTTP_REASON_BAD_REQUEST);
            response.setContentLength(0);
            response.send();
        }
        
    }
    catch(Exception &e) {
        poco_error_f1(Logger::get("RequestAnalyzerRemovePlaceParameter"), "exception [%s]", e.displayText());
    }
    catch(exception &e) {
        poco_error_f1(Logger::get("RequestAnalyzerRemovePlaceParameter"), "exception [%s]", string(e.what()));
    }

}









//============================================================================//
RequestAnalyzerInvokeMethod::RequestAnalyzerInvokeMethod(SharedPtr<Analyzer> analyzer) : _analyzer(analyzer) { }

//============================================================================//
RequestAnalyzerInvokeMethod::~RequestAnalyzerInvokeMethod() { }

//============================================================================//
void RequestAnalyzerInvokeMethod::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{

    try {
        //response.setChunkedTransferEncoding(true);
        response.setContentType("application/json; charset=utf-8");
        response.set("Connection", "close");
        response.set("Pragma", "no-cache");

        HTMLForm params;
        params.setFieldLimit(4096);
        params.load(request);

        if(params.has("method")) {
            string result, argument = params.has("arg") ? params["arg"] : string("");
            result = _analyzer->invokeCallback(params["method"], argument);

//            if(!result.size() || !result.substr(0, 3).compare("!!!")) {
//                response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST, HTTPResponse::HTTP_REASON_BAD_REQUEST);
//            }

            std::ostream& ostr = response.send();
            RequestAnalyzerJSONPCallback::begin(params, ostr);

            SharedPtr<DataEmitter> emitter = new JsonDataEmitter(ostr);

            emitter->list();
                emitter->dict();
                    emitter->key("result");
                    emitter->item(result);
                emitter->end();
            emitter->end();

            RequestAnalyzerJSONPCallback::end(params, ostr);
        }
        else {
            response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST, HTTPResponse::HTTP_REASON_BAD_REQUEST);
            response.setContentLength(0);
            response.send();
        }
    }
    catch(Exception &e) {
        poco_error_f1(Logger::get("RequestAnalyzerInvokeMethod"), "exception [%s]", e.displayText());
    }
    catch(exception &e) {
        poco_error_f1(Logger::get("RequestAnalyzerInvokeMethod"), "exception [%s]", string(e.what()));
    }

}



//============================================================================//
RequestAnalyzerSetConfig::RequestAnalyzerSetConfig(SharedPtr<Analyzer> analyzer) : _analyzer(analyzer) { }

//============================================================================//
RequestAnalyzerSetConfig::~RequestAnalyzerSetConfig() { }

//============================================================================//
void RequestAnalyzerSetConfig::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{

    try {
        //response.setChunkedTransferEncoding(true);
        response.setContentType("application/json; charset=utf-8");
        response.set("Connection", "close");
        response.set("Pragma", "no-cache");

        HTMLForm params;
        params.setFieldLimit(4096);
        params.load(request);

        HTMLForm::ConstIterator configIterator;

        int processedCount = 0;

        for(configIterator = params.begin(); configIterator != params.end(); ++configIterator) {

            if(!_analyzer->checkAllowedConfig(configIterator->first, configIterator->second)) continue;

            try {
                _analyzer->storage()->begin();
                _analyzer->storage()->setConfig(configIterator->first, configIterator->second, true);
                _analyzer->storage()->commit();
                processedCount += 1;

                _analyzer->invokeCallback("_onSetConfig", string(configIterator->first).append("`").append(configIterator->second));
            }
            catch(Exception &e) {
                _analyzer->storage()->rollback();
                response.setStatusAndReason(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR, HTTPResponse::HTTP_REASON_INTERNAL_SERVER_ERROR);
                response.setContentLength(0);
                response.send();
                return;
            }

        }

        std::ostream& ostr = response.send();
        RequestAnalyzerJSONPCallback::begin(params, ostr);

        SharedPtr<DataEmitter> emitter = new JsonDataEmitter(ostr);

        emitter->list();
            emitter->dict();
                emitter->key("processed");
                emitter->item(processedCount);
            emitter->end();
        emitter->end();

        RequestAnalyzerJSONPCallback::end(params, ostr);

    }
    catch(Exception &e) {
        poco_error_f1(Logger::get("RequestAnalyzerSetConfig"), "exception [%s]", e.displayText());
    }
    catch(exception &e) {
        poco_error_f1(Logger::get("RequestAnalyzerSetConfig"), "exception [%s]", string(e.what()));
    }

}
