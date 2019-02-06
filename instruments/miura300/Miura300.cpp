
#include <common/stdafx.h>
#include <sstream>

#include <Poco/ClassLibrary.h>
#include <Poco/LocalDateTime.h>
#include <Poco/StringTokenizer.h>


#include "Miura300.h"


#include "../../common/StringTools.h"
#include "../../analyzer/AnalyzerStorage.h"


using namespace std;


using namespace Poco;


//============================================================================//
Miura300::Miura300(Arguments &args) :
    Analyzer(args), transfer(), _isRequestResults(false), _isSendWorksheet(false)
{
    _callbacks.insert(make_pair("_onSetConfig", (AnalyzerCallback)&Miura300::onSetConfig));
    _callbacks.insert(make_pair("_onRemoveSample", (AnalyzerCallback)&Miura300::onRemoveSample));
    _callbacks.insert(make_pair("sendWorksheet", (AnalyzerCallback)&Miura300::sendWorksheet));
    _callbacks.insert(make_pair("receiveResults", (AnalyzerCallback)&Miura300::receiveResults));

    _allowModifyConifgs.push_back(make_pair("places\\.methods\\.[0-9a-z]{2,3}", 
        "^[A-Z0-9_]{2,32};[A-Z0-9_]{1,10};[01];[\\d]{1,3}$|^$")
    );
    
    try {
        _storage->begin();

        // setup places
        generatePlaces(1, 59);

        // setup default
        _storage->setConfig("information.storage.version", 		ANALYZERS_STORAGE_VERSION_STRING);
        _storage->setConfig("information.details.samples.supplier", 	"manual");
        _storage->setConfig("information.response.emptyResults", 	"false");
        
        // setup dicts for methods
        
        _storage->setConfig("places.methods.1a", "BIB_AMILAZA;AMY;0;0");
        _storage->setConfig("places.methods.1d", "BIB_AST;AST;0;0");
        _storage->setConfig("places.methods.1e", "BIB_GGTP;GGT;0;0");
        _storage->setConfig("places.methods.1i", "BIB_LDG;LDH;0;0");
        _storage->setConfig("places.methods.1j", "BIB_SHEL_FOSFOT;ALP;0;0");
        _storage->setConfig("places.methods.2a", "BIB_ALBUMIN;ALB;0;0");
        _storage->setConfig("places.methods.2b", "BIB_OBSH_BELOK;PROT;0;0");
        _storage->setConfig("places.methods.2c", "BIB_BILIRUB_OBSH;BILT;0;0");
        _storage->setConfig("places.methods.2d", "BIB_BILIRUB_PRYAM;BILD;0;0");
        _storage->setConfig("places.methods.2e", "BIB_GLUKOZA;GLU;0;0");
        _storage->setConfig("places.methods.2g", "BIB_KREATININ;CREA;0;0");
        _storage->setConfig("places.methods.2h", "BIB_MOCH_KISL;UA;0;0");
        _storage->setConfig("places.methods.2i", "BIB_MOCHEV;UREA;0;0");
        _storage->setConfig("places.methods.3a", "BIB_TRIGLICER;TG;0;0");
        _storage->setConfig("places.methods.3b", "BIB_HOLEST;CHOL;0;0");
        _storage->setConfig("places.methods.4a", "BIB_JELEZO;IRON;0;0");
        _storage->setConfig("places.methods.4b", "BIB_KALCIY;CALC;0;0");
        _storage->setConfig("places.methods.4d", "BIB_FOSFOR;PHOS;0;0");
        _storage->setConfig("places.methods.4e", "BIB_HLORID;CLOR;0;0");

        _storage->setParameter("2c", "RESEARCH_BIOCHEM_BLOOD_ANALYSIS", "BIB_CHEM_PROPERTIES",
                    "BIB_BILIRUB_OBSH", 0);
        _storage->setParameter("1a", "RESEARCH_BIOCHEM_BLOOD_ANALYSIS", "BIB_CHEM_PROPERTIES",
                    "BIB_AMILAZA", 60);
        _storage->setParameter("2d", "RESEARCH_BIOCHEM_BLOOD_ANALYSIS", "BIB_CHEM_PROPERTIES",
                    "BIB_BILIRUB_PRYAM", 10);
        
        _storage->setParameter("1d", "RESEARCH_BIOCHEM_BLOOD_ANALYSIS", "BIB_CHEM_PROPERTIES",
                    "BIB_AST", 20);
        
        _storage->setParameter("1e", "RESEARCH_BIOCHEM_BLOOD_ANALYSIS", "BIB_CHEM_PROPERTIES",
                    "BIB_GGTP", 40);
        
        _storage->setParameter("1i", "RESEARCH_BIOCHEM_BLOOD_ANALYSIS", "BIB_CHEM_PROPERTIES",
                    "BIB_LDG", 50);
        
        
        _storage->setParameter("1j", "RESEARCH_BIOCHEM_BLOOD_ANALYSIS", "BIB_CHEM_PROPERTIES",
                    "BIB_SHEL_FOSFOT", 70);
        
        _storage->setParameter("2b", "RESEARCH_BIOCHEM_BLOOD_ANALYSIS", "BIB_CHEM_PROPERTIES",
                    "BIB_OBSH_BELOK", 80);
        
        _storage->setParameter("2a", "RESEARCH_BIOCHEM_BLOOD_ANALYSIS", "BIB_CHEM_PROPERTIES",
                    "BIB_ALBUMIN", 90);
        
        _storage->setParameter("2e", "RESEARCH_BIOCHEM_BLOOD_ANALYSIS", "BIB_CHEM_PROPERTIES",
                    "BIB_GLUKOZA", 100);
        
        _storage->setParameter("2i", "RESEARCH_BIOCHEM_BLOOD_ANALYSIS", "BIB_CHEM_PROPERTIES",
                    "BIB_MOCHEV", 110);
        
        _storage->setParameter("2g", "RESEARCH_BIOCHEM_BLOOD_ANALYSIS", "BIB_CHEM_PROPERTIES",
                    "BIB_KREATININ", 120);
        
        _storage->setParameter("2h", "RESEARCH_BIOCHEM_BLOOD_ANALYSIS", "BIB_CHEM_PROPERTIES",
                    "BIB_MOCH_KISL", 130);
        
        _storage->setParameter("3b", "RESEARCH_BIOCHEM_BLOOD_ANALYSIS", "BIB_CHEM_PROPERTIES",
                    "BIB_HOLEST", 140);
        
        _storage->setParameter("3a", "RESEARCH_BIOCHEM_BLOOD_ANALYSIS", "BIB_CHEM_PROPERTIES",
                    "BIB_TRIGLICER", 150);
        
        _storage->setParameter("4a", "RESEARCH_BIOCHEM_BLOOD_ANALYSIS", "BIB_CHEM_PROPERTIES",
                    "BIB_JELEZO", 180);
        
        _storage->setParameter("4b", "RESEARCH_BIOCHEM_BLOOD_ANALYSIS", "BIB_CHEM_PROPERTIES",
                    "BIB_KALCIY", 190);
        
        _storage->setParameter("4d", "RESEARCH_BIOCHEM_BLOOD_ANALYSIS", "BIB_CHEM_PROPERTIES",
                    "BIB_FOSFOR", 200);
        
        _storage->setParameter("4e", "RESEARCH_BIOCHEM_BLOOD_ANALYSIS", "BIB_CHEM_PROPERTIES",
                    "BIB_HLORID", 210);
        
        _storage->commit();
    }
    catch(Exception &e) {
        _storage->rollback();
    }
}

//============================================================================//
Miura300::~Miura300()
{
}

//============================================================================//
string Miura300::onRemoveSample(const string &a)
{
    try {

        vector<RowAnalyzerPlace>    places;
        RowAnalyzerPlace::iter      placeIter;

        vector<RowAnalyzerSample>   samples;
        RowAnalyzerSample::iter     sampleIter;

        _storage->querySample(samples, 0, a);
        
        if(samples.size()) {
            
            _storage->queryPlace(places, "");
            
            int placeCodeIndex = 0;
            for(placeIter = places.begin(); placeIter != places.end(); ++placeIter, ++placeCodeIndex) {
                if(placeIter->id == samples[0].place_id) break;
            }
            
            _storage->begin();

            // clean methods
            _storage->removePlaceParameter(samples[0].place_id, "places.methods.%");
            
            // clean busy cell
            _storage->setVar(placeCodeIndex + 100000, "");
            
            _storage->commit();
        }
    }
    catch(Exception &e) {
        _storage->rollback();
        poco_error_f1(Logger::get("Miura300"),
            "fail analyzer main loop, %s", e.displayText());
        stop();
        return "!!!FAIL";
    }
    catch(exception &e) {
        _storage->rollback();
        poco_error_f1(Logger::get("Miura300"),
            "fail analyzer main loop, %s", string(e.what()));
        stop();
        return "!!!FAIL";
    }
    
    return "";
}

//============================================================================//
string Miura300::onSetConfig(const string &a)
{
    try {
        if(!a.find("places.methods.")) {
        
            string valueConfig;
            string nameConfig;
            
            int valueIndex = a.find('`');
            
            if(valueIndex > 0) {
                nameConfig = a.substr(0, valueIndex);
                valueConfig = a.substr(valueIndex + 1);
            }
            else {
                return "!!!FAIL";
            }
            
            _storage->begin();
            
            StringTokenizer tokens(valueConfig, ";");

            if(tokens.count() == 0) {
                return "!!!FAIL";
            }

            string barcode  = nameConfig.substr(nameConfig.rfind('.') + 1);
            string testcode = tokens[0];

            _storage->setParameter(barcode, "RESEARCH_BIOCHEM_BLOOD_ANALYSIS", "BIB_CHEM_PROPERTIES",
                testcode, 300, 1, "", true);

            _storage->commit();
            
        }
    }
    catch(Exception &e) {
        _storage->rollback();
        poco_error_f1(Logger::get("Miura300"),
            "fail analyzer main loop, %s", e.displayText());
        stop();
        return "!!!FAIL";
    }
    catch(exception &e) {
        _storage->rollback();
        poco_error_f1(Logger::get("Miura300"),
            "fail analyzer main loop, %s", string(e.what()));
        stop();
        return "!!!FAIL";
    }
    
    return "";
}

//============================================================================//
string Miura300::sendWorksheet(const string &a)
{
    if(_isSendWorksheet) {
        return "process";
    }
    else {
        string result;
        //if(_storage->waitTransaction()) {
        _storage->begin();
        _storage->getVar(10000, result);
        _storage->commit();
        //}
        
        if(result.size()) {
            _storage->begin();
            _storage->setVar(10000, "");
            _storage->commit();
            return result;
        }
        
        _isSendWorksheet = true;
    }
    return "run";
}

string Miura300::sendWorksheetTask()
{
    bool itWasBeginRequest = false;
    
    try {
        ObjectAnalyzerPlace::iter       placeIter;
        vector<ObjectAnalyzerPlace>     places;

        int                             startId = 100000;
        map<int, string>                innerVars;
        map<int, string>::iterator      innerVar;
        
        // query samples
        _storage->queryPlaceDetailed(places, "", true, true);
        _storage->getVar(startId, startId + places.size(), innerVars);
        
        try {
            transfer->beginRequest();
            itWasBeginRequest = true;
        }
        catch(astm::AstmException &e) {
            throw;
        }
        catch(exception &e) {
            return "!!!FAIL TO CONNECT";
        }
        
        astm::AstmRecordHeader      header;
        header.processingId.setString("P");
        header.sender.setString("SmitLab");
        header.version.setString("LIS2-A2");
        header.timestamp.setDatetime(LocalDateTime());

        astm::AstmRecordTerminator  terminator;
        terminator.code.setString("N");
        
        astm::AstmFieldDelimiters   delimiters;
       
        transfer->writeRecord(header, delimiters);
        
        int seqPatient = 0, placeIndex = 0;
        for(placeIter = places.begin(); placeIter != places.end(); ++placeIter, ++placeIndex) {
            
            int seqOrder = 0;
            
            if(placeIter->samples.size()) {
                
                if(!placeIter->parameters.size()) {
                    continue;
                }
                
                innerVar = innerVars.find(startId + placeIndex);
                
                if(innerVar != innerVars.end() && !innerVar->second.compare(placeIter->samples[0].uid)) {
                    continue;
                }
                
                astm::AstmRecordPatient     patient;
                
                patient.seq = ++seqPatient;
                
                patient.laboratoryId.setString(placeIter->code + "_" + placeIter->samples[0].uid);
                patient.name.setString(placeIter->samples[0].patient_uid);
                patient.sex.setString(placeIter->samples[0].patient_sex);
                patient.dosageCategory.setString("A");
                // patient.race.setString("W");

                transfer->writeRecord(patient, delimiters);
                
                for(RowAnalyzerPlaceParameter::iter ppIter = placeIter->parameters.begin();
                        ppIter != placeIter->parameters.end();
                        ++ppIter) {
                    
                    if(ppIter->code.find("places.methods.")) continue;
                    
                    astm::AstmRecordOrder       order;
                    order.test.setComponentSize(4);
                    
                    order.seq = ++seqOrder;

                    // not used
                    order.sampleId.setString(patient.laboratoryId.asString());
                    
                    // required labid
                    order.instrument.setString(patient.laboratoryId.asString());
                    
                    order.test.setString(ppIter->code.substr(15), 1);
                    order.test.setString(ppIter->value.size() ? ppIter->value : string("1"), 3);

                    // routine
                    order.priority.setString("R");
                    order.createdAt.setDatetime(LocalDateTime());

                    // new order
                    order.actionCode.setString("N");

                    // @TODO: Make sample type
                    order.biomaterial.setString("S");
                    order.reportType.setString("O");
                    
                    transfer->writeRecord(order, delimiters);
                }
                
                // @TODO: Make more flex
                // Prevent order duplicity send
                _storage->setVar(startId + placeIndex, placeIter->samples[0].uid);
            }
        }
        
        transfer->writeRecord(terminator, delimiters);
        
    }
    catch(astm::AstmStateException &e) {
        return string("!!!") + e.what() + string(" ") + e.displayMessage();
    }
    catch(astm::AstmFrameException &e) {
        return string("!!!") + e.what() + string(" ") + e.displayMessage();
    }
    catch(Exception &e) {
        poco_error_f1(Logger::get("Miura300"),
            "fail analyzer main loop, %s", e.displayText());
        stop();
        return "!!!FAIL";
    }
    catch(exception &e) {
        poco_error_f1(Logger::get("Miura300"),
            "fail analyzer main loop, %s", string(e.what()));
        stop();
        return "!!!FAIL";
    }
    if(itWasBeginRequest)
        transfer->endRequest();
    
    return "sendWorksheet";
}

// 186042
// 183370
// 184766

// 186482
// 120490


//============================================================================//
string Miura300::receiveResults(const string &a)
{
    if(_isRequestResults) {
        return "process";
    }
    else {
        string result;
        //if(_storage->waitTransaction()) {
        _storage->begin();
        _storage->getVar(10001, result);
        _storage->commit();
        //}
        
        if(result.size()) {
            _storage->begin();
            _storage->setVar(10001, "");
            _storage->commit();
            return result;
        }
        
        _isRequestResults = true;
    }
    return "run";
}


string Miura300::receiveResultsTask()
{
    bool itWasBeginRequest = false, 
         oneMoreResult = false,
         wasTerminator = false;
    
    try {
        
        vector<RowAnalyzerPlace>    places;
        RowAnalyzerPlace::iter      pIter;

        vector<RowAnalyzerSample>   samples;
        RowAnalyzerSample::iter     sIter;

        vector<RowAnalyzerPlaceParameter>   placeParams;
        RowAnalyzerPlaceParameter::iter     ppIter;

        vector<RowAnalyzerConfig>   conifgs;
        RowAnalyzerConfig::iter     cIter;

        //_storage->queryPlace(places, "");
        //_storage->queryConfig(conifgs, "methods.parameters.%");
        
        // prepare methods to parameters map
        map<string, int>::iterator   parameter;
        
//        ObjectAnalyzerPlace::iter       placeIter;
//        vector<ObjectAnalyzerPlace>     places;
//
//        int                             startId = 100000;
//        map<int, string>                innerVars;
//        map<int, string>::iterator      innerVar;
//        
//        // query samples
//        _storage->queryPlaceDetailed(places, "", true, true);
//        _storage->getVar(startId, startId + places.size(), innerVars);
//        
        try {
            transfer->beginRequest();
            itWasBeginRequest = true;
        }
        catch(astm::AstmException &e) {
            throw;
        }
        catch(exception &e) {
            return "!!!FAIL TO CONNECT";
        }

        astm::AstmRecordHeader      sendHeader;
        astm::AstmRecordHeader      receiveHeader;
        astm::AstmRecordRequest     sendRequest;

        sendHeader.processingId.setString("P");
        sendHeader.sender.setString("SmitLab");
        sendHeader.version.setString("LIS2-A2");
        sendHeader.timestamp.setDatetime(LocalDateTime());

        //sendRequest.startId.setString("0");
        //sendRequest.timeStart.setString("20140331000000");
        sendRequest.status.setString("F");
        //sendRequest.timeStart.setString("ALL");

        astm::AstmFieldDelimiters   sendDelims, receiveDelims;

        // transfer->writeRecord(sendHeader, sendDelims);
        transfer->writeRecord(sendRequest, sendDelims);
        
        int prevRecType = 0;
        string frameData;

        string sampleUid = "";

        ObjectAnalyzerResult::iter          resultIter;
        vector<ObjectAnalyzerResultGroup>   resultGruops;
        int resultGroupId;
        
        do {

            transfer->receiveFrameData(frameData);

            if(astm::AstmRecordHeader::isType(frameData)) {
                receiveHeader.decode(frameData, receiveDelims);
                prevRecType = receiveHeader.recordType;
            }
            else if(astm::AstmRecordTest::isType(frameData) && sampleUid.size()) {
                astm::AstmRecordTest r;
                r.test.setComponentSize(4);
                r.decode(frameData, receiveDelims);

                // skip not finished results
                if(r.status.asString().compare("F"))
                    continue;

                // method name
                parameter = _parameterMapper.find(r.test.asString(1));

                if(parameter == _parameterMapper.end()) {
                    poco_warning_f1(Logger::get("Miura300"),
                        "unknown received order result [%s]", r.test.asString(1));
                    continue;
                }
                
                // find exist result
                bool isResultExist = false;
                if(resultGruops.size() && resultGruops[0].results.size() > 0) {
                    for(resultIter = resultGruops[0].results.begin();
                            resultIter != resultGruops[0].results.end();
                            ++resultIter)
                    {
                        if(!resultIter->parameter.code.compare(r.test.asString(1))) {
                            isResultExist = true;
                            break;
                        }
                    }
                    
                    if(isResultExist) {
                        // break;
                    }
                }
                
                try {
                    // value
                    double resultDbl = 0;
                    string resultStr;
                    string units = r.units.isEmpty() ? "" : r.units.asString();
                    string flags = r.abnormalFlag.isEmpty() ? "" : r.abnormalFlag.asString();

                    try {
                        resultDbl = r.value.asFloat();

                        format(resultStr, "%.3f", resultDbl);
                    }
                    catch(exception &e) {
                        resultStr = r.value.asString();
                    }

                    common::trimRightZeroes(resultStr);

                    // add new result
                    try {
                        _storage->begin();
                        _storage->setResult(resultGroupId, parameter->second,
                                units, resultStr, flags, "ok", "");
                        
                        if(!oneMoreResult) {
                            oneMoreResult = true;
                            _storage->setSampleStatus(sampleUid, "groups", "results");
                        }
                        _storage->commit();
                    }
                    catch(exception &e) {
                        _storage->rollback();
                    }
                    
                }
                catch(exception &e) {
                }
                
                prevRecType = r.recordType;
            }
            else if(astm::AstmRecordOrder::isType(frameData)) {
                astm::AstmRecordOrder r;
                r.decode(frameData, receiveDelims);
                prevRecType = r.recordType;
            }
            else if(astm::AstmRecordPatient::isType(frameData)) {
                oneMoreResult = false;
                astm::AstmRecordPatient r;
                r.decode(frameData, receiveDelims);
                prevRecType = r.recordType;

                sampleUid = r.laboratoryId.asString();
                
                int pos;
                if((pos = sampleUid.find('_')) > -1) {
                    sampleUid = sampleUid.substr(pos + 1);
                }
                
                // find sample
                samples.clear();
                _storage->querySample(samples, 0, sampleUid);
                if(!samples.size()) {
                    sampleUid = "";
                    continue;
                }
                
                // find results
                resultGruops.clear();
                _storage->queryResultGroup(resultGruops, "", 0, sampleUid);
                
                if(!resultGruops.size()) {
                    _storage->begin();
                    resultGroupId = _storage->addResultGroup(samples[0].place_id, 0, sampleUid);
                    _storage->commit();
                    
                    if(!resultGroupId) {
                        sampleUid = "";
                        continue;
                    }
                }
                else {
                    resultGroupId = resultGruops[0].id;
                }
                    
                
                // sampleUid = r.laboratoryId.asString();
            }
            else if (astm::AstmRecordTerminator::isType(frameData)) {
                wasTerminator = true;
            }

        } while(frameData.size());
     
        if(wasTerminator) {
            poco_information(Logger::get("Miura300"), "results received, send [EOT]");
        }
        else {
            poco_warning(Logger::get("Miura300"), "break transaction, no termination record was accept!");
            return string("!!!BREAK TRANSACTION");
        }
    }
    catch(astm::AstmStateException &e) {
        return string("!!!") + e.what() + string(" ") + e.displayMessage();
    }
    catch(astm::AstmFrameException &e) {
        return string("!!!") + e.what() + string(" ") + e.displayMessage();
    }
    catch(Exception &e) {
        poco_error_f1(Logger::get("Miura300"),
            "fail analyzer main loop, %s", e.displayText());
        stop();
        return "!!!FAIL";
    }
    catch(exception &e) {
        poco_error_f1(Logger::get("Miura300"),
            "fail analyzer main loop, %s", string(e.what()));
        stop();
        return "!!!FAIL";
    }
    if(itWasBeginRequest) {
        if(wasTerminator) {
            transfer->endResponse();
        }
        else {
            transfer->endRequest();
        }
    }
    
    
    return "receiveResults";
}

//============================================================================//
void Miura300::run()
{
    Analyzer::run();

    // prepare io
    transfer = new astm::AstmTransfer(*_io);
    while(!_isTermination) {
        
        // 
        // Periodic request!?
        //
        try {
            
            if(_isSendWorksheet) {
                string result = sendWorksheetTask();
                try {
                    //if(_storage->waitTransaction()) {
                    _storage->begin();
                    _storage->setVar(10000, result);
                    _storage->commit();
                    //}
                }
                catch(exception &e) {
                    _storage->rollback();
                }
                _isSendWorksheet = false;
            }
            
            if (_isRequestResults) {
                string result = receiveResultsTask();
                try {
                    //if(_storage->waitTransaction()) {
                    _storage->begin();
                    _storage->setVar(10001, result);
                    _storage->commit();
                    //}
                }
                catch(exception &e) {
                    _storage->rollback();
                }
                _isRequestResults = false;
            }
        }
        catch(exception &e) {
        }
        
        _thread.sleep(500);
    }

}

//============================================================================//
POCO_BEGIN_MANIFEST(Analyzer)
POCO_END_MANIFEST

void pocoInitializeLibrary()    { }
void pocoUninitializeLibrary()  { }

int registerInstrument(AnalyzerDispatcher &dispatcher)
{
    dispatcher.registerAnalyzerFactory<Miura300>("miura300");
    return 0;
}

int unregisterInstrument(AnalyzerDispatcher &dispatcher)
{
    return 0;
}
