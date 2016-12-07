
#include <common/stdafx.h>


#include <Poco/Format.h>
#include <Poco/RegularExpression.h>
#include <Poco/NumberParser.h>

#include <Poco/Path.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/DatagramSocket.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Data/Session.h>
#include <Poco/Util/ConfigurationView.h>


#include "../common/IOAdapter.h"


#include "Analyzer.h"
#include "AnalyzerReader.h"
#include "AnalyzerStorage.h"


using namespace std;


using namespace Poco;
using namespace Poco::Net;
using namespace Poco::Util;


//============================================================================//
const char *__analyzerLoggerName__ = "Analyzer";


//============================================================================//
Analyzer::Analyzer(Arguments &args) :
    _code(args.code),
    _config(args.config),
    _storage(new AnalyzerStorage(
        Path(args.config.getString("analyzers.storages.path", ""), args.code + ".storage").toString(),
        args.config.getBool("analyzers.storages.extends", false))
    ),
    _subConfig(new ConfigurationView("analyzers.list." + args.code, &args.config)),
    _isTermination(false),
    _thread(args.code),
    _io()
{
}

//============================================================================//
Analyzer::~Analyzer()
{
}

//============================================================================//
void Analyzer::getCallbacks(vector<string> &callbacks)
{
    CallbackIterator iter;
    for(iter = _callbacks.begin();
            iter != _callbacks.end();
            ++iter) 
    {
        if(iter->first[0] == '_') continue;
        callbacks.push_back(iter->first);
    }
}

//============================================================================//
string Analyzer::invokeCallback(const string &name, const string &arg)
{
    ScopedLock<FastMutex> _scope(_mutex);

    CallbackIterator iter = _callbacks.find(name);

    if(iter != _callbacks.end()) {
        return (this->*iter->second)(arg);
    }

    return "!!!UNKNOWN_METHOD!!!";
}

//============================================================================//
bool Analyzer::checkAllowedConfig(const string &name, const string &value) const
{
    bool allowConfig = false;

    for(vector<pair<string, string> >::const_iterator allow = _allowModifyConifgs.begin();
        allow != _allowModifyConifgs.end();
        ++allow) {

        // cout << allow->first << " " << allow->second << " " << name << " " << value << endl;

        RegularExpression nameRegExp(allow->first);
        if(!nameRegExp.match(name)) {
            continue;
        }

        RegularExpression valueRegExp(allow->second);
        if(valueRegExp.match(value, 0, 0)) {
            allowConfig = true;
        }

    }

    return allowConfig;
}

//============================================================================//
string Analyzer::getCode()
{
    return _code;
}

//============================================================================//
string Analyzer::getType()
{
    return _subConfig->getString("type");
}

//============================================================================//
string Analyzer::getTitle()
{
    return _subConfig->getString("title");
}


//============================================================================//
const string Analyzer::getResultPlaceIdentifier()
{
    vector<RowAnalyzerConfig> configs;
    RowAnalyzerConfig::iter   cb, ce;
    string                    lastIndex = "000000";
    string                    defaultPlace = "";

    _storage->queryConfig(configs, "settings.places.");

    cb = configs.begin(); ce = configs.end();
    // enumerate configs and act on force action
    while(cb != ce) {
        if(!cb->code.compare("settings.places.default"))         { defaultPlace = cb->value; }
        else if(!cb->code.compare("settings.places.force_to") && cb->code.size())   {
            _storage->setConfig("settings.places.force_to", "", true);
            return cb->value; 
        }
        ++cb;
    }

    vector<ObjectAnalyzerPlace> places;
    ObjectAnalyzerPlace::iter   pb, pe;
    ObjectAnalyzerSample::iter  sb, se;

    _storage->queryPlaceDetailed(places, "%", false, false);

    pb = places.begin(); pe = places.end();

    // iterate over places and find free place otherwise it will be last index
    while(pb != pe) {

        // iterate over samples
        if(pb->samples.size()) {
            sb = pb->samples.begin(); se = pb->samples.end();

            while(sb != se) {

                // check results
                if(!sb->groups.size()) {
                    return pb->code;
                }

                ++sb;
            }

        }
        // check results
        else if(!pb->groups.size()) {
            return pb->code;
        }

        if(lastIndex.compare(pb->code) < 0) {
            lastIndex = pb->code;
        }

        ++pb;
    }

    if(defaultPlace.size()) {
        return defaultPlace;
    }

    return lastIndex;
}

//============================================================================//
SharedPtr<AnalyzerStorage> Analyzer::storage()
{
    return _storage;
}

//============================================================================//
void Analyzer::start()
{
    stop();

    // @TODO: Place it to thread
    openCommunication();

    _thread.start(*this);

    poco_information(Logger::get(__analyzerLoggerName__), _code + " started");
}


//============================================================================//
void Analyzer::stop()
{
    if(_thread.isRunning()) {

        _isTermination = true;
        _thread.join();

        poco_information(Logger::get(__analyzerLoggerName__), _code + " close connection...");

        closeCommunication();

        poco_information(Logger::get(__analyzerLoggerName__), _code + " stopped");
    }

}

//============================================================================//
void Analyzer::openCommunication()
{
    try {
        poco_information(Logger::get(getCode()), "open communication");

        if(!_subConfig->getString("communication.type", "dummy").compare("serialport")) {

            poco_information_f1(Logger::get(__analyzerLoggerName__),
                "use serialport connection [%s]", getCode());

            _io = new common::IOAdapterSerial(*_subConfig);
        }
        else if(!_subConfig->getString("communication.type", "dummy").compare("ethernet")) {

            bool        isTcp= !_subConfig->getString("communication.ethernet_type", "tcp").compare("tcp");

            if(isTcp) {
                poco_information_f1(Logger::get(__analyzerLoggerName__),
                    "use tcp ethernet connection [%s]", getCode());
            }
            else {
                poco_information_f1(Logger::get(__analyzerLoggerName__),
                    "use udp ethernet connection [%s]", getCode());
            }

            _io = new common::IOAdapterEthernet(*_subConfig);
        }

    }
    catch(Exception &e) {
        poco_error_f1(Logger::get(__analyzerLoggerName__),
            "fail to open communication [%s]", e.displayText());
        e.rethrow();
    }
    catch(exception &e) {
        poco_error_f1(Logger::get(__analyzerLoggerName__),
            "fail to open communication [%s]", string(e.what()));
        throw e;
    }

}

//============================================================================//
void Analyzer::closeCommunication()
{

    poco_information(Logger::get(__analyzerLoggerName__), "close communication");
    // _io = NULL;
}

//============================================================================//
void Analyzer::run()
{

    _isTermination = false;

    // fill mapping of parameters.
    if(!_parameterMapper.size()) {

        vector<RowAnalyzerParameter>       params;
        _storage->queryParameter(params, "");

        RowAnalyzerParameter::iter  param = params.begin(), paramEnd = params.end();
        while(param != paramEnd) {
            _parameterMapper.insert(pair<string,int>(param->code, param->id));
            ++param;
        }

    }
}

//============================================================================//
void Analyzer::processSingleResult()
{

    static int fastTimeout = 500;
    static int slowTimeout = 5000;

    _io->setReadTimeout(fastTimeout);

    if(_resultReader->isInstrumentSentResult()) {

        poco_information(Logger::root(), "receive results");

        _io->setReadTimeout(slowTimeout);

        ObjectAnalyzerResultGroup   result;

        // receive all results
        if(_resultReader->readInstrumentResult(result, _parameterMapper)) {

            poco_information(Logger::root(), "save results");

            try {

                _storage->begin();

                // query place
                vector<RowAnalyzerPlace> places;
                string placeCode = this->getResultPlaceIdentifier();
                _storage->queryPlace(places, placeCode);

                poco_debug_f1(Logger::root(), "put results into specified place [%s]", placeCode);
                if(!places.size()) {
                    poco_warning_f1(Logger::root(), "fail to place results into specified place [%s]", placeCode);
                    return;
                }

                int placeId = places[0].id;

                // query sample
                vector<RowAnalyzerSample> samples;
                _storage->querySample(samples, placeId, "");

                // add result group
                string sampleUid = "";
                if(samples.size() > 0) {
                    sampleUid = samples[0].uid;
                    _storage->setSampleStatus(sampleUid, "groups", "results");
                }

                int resultGroupId = _storage->addResultGroup(placeId, 0, sampleUid);
                _storage->setResultGroupStatus(resultGroupId, result.status, result.comment);

                ObjectAnalyzerResult::iter  ir = result.results.begin(), er = result.results.end();

                while(ir != er) {

                    _storage->setResult(resultGroupId, ir->parameter_id,
                        ir->units, ir->val, ir->flags, ir->status, ir->comment);

                    ++ir;

                }

                _storage->commit();
            }
            catch(Exception &e) {
                poco_warning(Logger::root(), "fail to save results " + e.displayText());
                _storage->rollback();
            }
            catch(exception &e) {
                poco_warning(Logger::root(), "fail to save results" + string(e.what()));
                _storage->rollback();
            }

        }
        else {
            poco_warning(Logger::root(), "fail to receive results");
        }

        _io->setReadTimeout(fastTimeout);
    }
}

//----------------------------------------------
void Analyzer::generatePlaces()
{
    vector<RowAnalyzerConfig>   placesCount;
    int                         from = 1, to = 1;

    _storage->queryConfig(placesCount, "settings.places.count");

    if(placesCount.size() == 1) {
        NumberParser::tryParse(placesCount[0].value, to);
    }

    generatePlaces(from, to);
}

//----------------------------------------------
void Analyzer::generatePlaces(const int &fromPlaceIndex, const int &toPlaceIndex)
{
    int placeIndex = fromPlaceIndex;

    char placeTitle[5];
    string place;

    for(;placeIndex <= toPlaceIndex; placeIndex += 1) {

        int size =

#if defined(POCO_OS_FAMILY_WINDOWS)
        _snprintf_s(placeTitle,
#else
        snprintf(placeTitle,
#endif
        4, "%d", placeIndex);

        placeTitle[size] = 0;

        place.clear();

        if(placeIndex < 1000)           place.append(1, '0');
        if(placeIndex < 100)            place.append(1, '0');
        if(placeIndex < 10)             place.append(1, '0');

        place.append(placeTitle);

        _storage->setPlace(place, placeTitle);
    }

}
