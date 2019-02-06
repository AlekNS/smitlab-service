
#include <common/stdafx.h>


#include <Poco/Format.h>
#include <Poco/NumberParser.h>
#include <Poco/Thread.h>
#include <Poco/String.h>


#include "LauraSmartResultReader.h"


using namespace std;


using namespace Poco;


using namespace common;


//============================================================================//
AnalyzerLauraSmartResultReader::AnalyzerLauraSmartResultReader(common::IOAdapter &_io, 
    AbstractConfiguration &subConfig) :
    AnalyzerResultReader(_io),
    _subConfig(subConfig)
{
    poco_information_f1(
        Logger::get("AnalyzerLauraSmart"),
        "Measure in [%s] units", _subConfig.getString("units.type", "si")
    );
}

AnalyzerLauraSmartResultReader::~AnalyzerLauraSmartResultReader()
{
}

bool AnalyzerLauraSmartResultReader::isInstrumentSentResult()
{
    char symbol = 0;
    try {
        _io.input->clear();
        if(_io.input->read(&symbol, 1)) {
            if(symbol == CharUtil::STX) {
                return true;
            }
            else {
                while(_io.input->read(&symbol, 1));
            }
        }
    }
    catch(exception &e) {
    }

    return false;
}


enum {
    LAURASTATE_WAIT_RESULT,
    LAURASTATE_SKIP_LINE,
    LAURASTATE_RESULTS,
    LAURASTATE_END_RESULT
};

bool AnalyzerLauraSmartResultReader::readInstrumentResult(ObjectAnalyzerResultGroup &result,
    map<string, int> &paramsMapper)
{

    try {

        _io.input->clear();

        char    receivedData[502] = {0};

        _io.input->getline(receivedData, 502, (char)CharUtil::ETX);

        char line[48] = {0};
        int  lineIndex = 0, state = LAURASTATE_WAIT_RESULT, stateNext = state;
        string code, value, units;
        bool    isAssigned, isNumber;
        bool    isMeasuredInSi = !_subConfig.getString("units.type", "si").compare("si");

        istringstream istr;
        istr.str(string(receivedData));

        while(istr.good()) {

                istr.getline(line, 48, (char)CharUtil::LF);
                

                switch(state) {
                    case LAURASTATE_WAIT_RESULT:
                        // stupid checking of result group begin
                        if(!strncmp(line, "........................", 24)) {
                            state = LAURASTATE_SKIP_LINE;
                            stateNext = LAURASTATE_RESULTS;
                        }
                        break;
                    case LAURASTATE_SKIP_LINE:
                        state = stateNext;
                        break;
                    case LAURASTATE_RESULTS:
                        if(!strncmp(line, "------------------------", 24)) {
                            state     = LAURASTATE_SKIP_LINE;
                            stateNext = LAURASTATE_END_RESULT;
                            break;
                        }

                        value.clear();
                        code.clear();
                        units.clear();

                        code.append(line + 1, 3);
                        value.append(line + 7, 5);

                        isAssigned = false;
                        isNumber   = false;
                        
                        if(code == "UV ") {
                            isAssigned  = true; isNumber    = true; units = "UNKNOWN";
                        }
                        else if(code == "LEJ") {
                            units = "LEJ_MKL";
                            if(isMeasuredInSi) {
                                units = "LEJ_MKL";
                            }
                            isAssigned  = true;
                            if(value == "   1+") {
                                value = isMeasuredInSi ? "25" :  "25";
                            }
                            else if(value == "   2+") {
                                value = isMeasuredInSi ? "75" :  "75";
                            }
                            else if(value == "   3+") {
                                value = isMeasuredInSi ? "500" : "500";
                            }
                            else {
                                value = "0";
                            }
                        }
                        else if(code == "NIT") {
                            isAssigned  = true;
                            if(value == "  POL")        value = "POLOJIT";
                            else                        value = "OTRIC";
                        }
                        else if(code == "rN ") {
                            isAssigned  = true;
                            isNumber    = true;
                        }
                        else if(code == "BEL") {
                            units = isMeasuredInSi ? "G_L" : "MG_DL";
                            isAssigned  = true;
                            isNumber    = true;
                            if(value == "   1+")        value = isMeasuredInSi ? "0.3" : "30";
                            else if(value == "   2+")   value = isMeasuredInSi ? "1"   : "100";
                            else if(value == "   3+")   value = isMeasuredInSi ? "5"   : "500";
                            else                        value = "0";
                        }
                        else if(code == "GL\xDA") {
                            units = isMeasuredInSi ? "MMOL_L" : "MG_DL";
                            isAssigned  = true;
                            isNumber    = true;
                            if(value == "   1+")        value = isMeasuredInSi ? "2.8" : "50";
                            else if(value == "   2+")   value = isMeasuredInSi ? "5.5" : "100";
                            else if(value == "   3+")   value = isMeasuredInSi ? "17"  : "300";
                            else if(value == "   4+")   value = isMeasuredInSi ? "55"  : "1000";
                            else                        value = "0";
                        }
                        else if(code == "KET") {
                            units = isMeasuredInSi ? "MMOL_L" : "MG_DL";
                            isAssigned  = true;
                            isNumber    = true;
                            if(value == "   +-")        value = isMeasuredInSi ? "0.5" : "5.2";
                            else if(value == "   1+")   value = isMeasuredInSi ? "1.5" : "16";
                            else if(value == "   2+")   value = isMeasuredInSi ? "5"   : "52";
                            else if(value == "   3+")   value = isMeasuredInSi ? "15"  : "156";
                            else                        value = "0";
                        }
                        else if(code == "UBG") {
                            units = isMeasuredInSi ? "MKMOL_L" : "MG_DL";
                            isAssigned  = true;
                            isNumber    = true;
                            if(value == "   1+")        value = isMeasuredInSi ? "17"  : "1";
                            else if(value == "   2+")   value = isMeasuredInSi ? "51"  : "3";
                            else if(value == "   3+")   value = isMeasuredInSi ? "102" : "6";
                            else if(value == "   4+")   value = isMeasuredInSi ? "203" : "12";
                            else                        value = "0";
                        }
                        else if(code == "BIL") {
                            units = isMeasuredInSi ? "MKMOL_L" : "MG_DL";
                            isAssigned  = true;
                            isNumber    = true;
                            if(value == "   1+")        value = isMeasuredInSi ? "17"  : "1";
                            else if(value == "   2+")   value = isMeasuredInSi ? "51"  : "3";
                            else if(value == "   3+")   value = isMeasuredInSi ? "103" : "6";
                            else                        value = "0";
                        }
                        else if(code == "KRV") {
                            units = "ERY_MKL";
                            if(isMeasuredInSi) {
                                units = "ERY_MKL";
                            }
                            isAssigned  = true;
                            if(value == "   1+") {
                                value = isMeasuredInSi ? "10" : "10";
                            }
                            else if(value == "   2+") {
                                value = isMeasuredInSi ? "50" : "50";
                            }
                            else if(value == "   3+") {
                                value = isMeasuredInSi ? "250" : "250";
                            }
                            else {
                                value = "0";
                            }
                        }
                        else {
                            poco_warning_f1(Logger::get("AnalyzerLauraSmart"),
                                "unknown analyzer code for [%s]", string(line));
                        }

                        if(isAssigned) {
                            bool isNumberParsed = false;
                            double dblValue;

                            if(isNumber && (isNumberParsed = NumberParser::tryParseFloat(value, dblValue))) {
                                value.clear();
                                format(value, "%f", dblValue);
                                trimRightZeroes(value);
                            }

                            map<string,int>::iterator ind = paramsMapper.find(code);
                            ObjectAnalyzerResult      r;

                            // in normal situation this is dummy code, but everything possible!
                            if(paramsMapper.end() == ind) {
                                result.status = "warning";
                                result.comment = "Не все параметры обработались правильно.";
                                r.status = "error";
                                r.comment = "Неизвестный системе параметр (";
                                r.comment += code;
                                r.comment += ")";
                                break;
                            }
                            else {
                                r.parameter_id = ind->second;
                                r.val          = trim(value);
                                r.flags        = "";
                                r.units        = trim(units);
                                r.status       = "ok";
                            }

                            result.results.push_back(r);
                        }

                    break;
                    default:
                    break;
                }

            lineIndex += 1;
        }
        result.status = "ok";
        
        if(!result.results.size()) {
            result.comment = "empty";
        }
        
        return true;
    }
    catch(Exception &e) {
        result.status = "error";
        result.comment = e.displayText();
    }
    catch(exception &e) {
        result.status = "error";
        result.comment = e.what();
    }

    return false;
}
