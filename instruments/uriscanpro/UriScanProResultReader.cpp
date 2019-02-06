
#include <common/stdafx.h>


#include <Poco/Format.h>
#include <Poco/NumberParser.h>
#include <Poco/Thread.h>
#include <Poco/String.h>
#include <Poco/StringTokenizer.h>


#include "UriScanProResultReader.h"


using namespace std;


using namespace Poco;


using namespace common;


//============================================================================//
AnalyzerUriScanProResultReader::AnalyzerUriScanProResultReader(common::IOAdapter &_io, 
        AbstractConfiguration &subConfig) :
    AnalyzerResultReader(_io),
    _subConfig(subConfig)
{
}

AnalyzerUriScanProResultReader::~AnalyzerUriScanProResultReader()
{
}

bool AnalyzerUriScanProResultReader::isInstrumentSentResult()
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
    URISCANPRO_WAIT_RESULT,
    URISCANPRO_SKIP_LINE,
    URISCANPRO_RESULTS,
    URISCANPRO_END_RESULT
};

bool AnalyzerUriScanProResultReader::readInstrumentResult(ObjectAnalyzerResultGroup &result,
    map<string, int> &paramsMapper)
{

    try {
        _io.input->clear();

        char    receivedData[687] = {0};

        _io.input->getline(receivedData, 687, (char)CharUtil::ETX);

        char line[48] = {0};
        int  lineIndex = 0, state = URISCANPRO_WAIT_RESULT;
        string code, value, units;
        bool   isAssigned, isNumber;

        StringTokenizer parsedItems(receivedData, "\n");
        if(parsedItems.count() != 17) {
            poco_error(Logger::get("AnalyzerUriScanPro"),
                "fail to parse received data, the parsed items count not equals to 17 count");
            return false;
        }

        istringstream istr;
        istr.str(string(receivedData));

        
        while(istr.good()) {
                 isNumber = false;
                 isAssigned = false;
                 value.clear();
                 units.clear();
                 code.clear();

                istr.getline(line, 48, (char)CharUtil::LF);
                line[47] = 0;

                StringTokenizer parsed(line, " ", StringTokenizer::TOK_IGNORE_EMPTY | StringTokenizer::TOK_TRIM);
                
                switch(state) {
                    case URISCANPRO_RESULTS:

                        switch(lineIndex) {
                            // blood
                            case 4:
                                isAssigned = true;
                                
                                if(parsed.count() < 4) throw Exception("parse blood");
                                value = parsed[3];
                                
                                code = "KRV"; units = "ERY_MKL";    isNumber = true;
                                if(!value.compare("neg")) {
                                    value = "0";
                                }
                                break;

                            // bil
                            case 5:
                                isAssigned = true;
                                
                                if(parsed.count() < 3) throw Exception("parse bilirub");
                                value = parsed[2];
                                
                                code = "BIL"; units = "MG_DL";      isNumber = true;
                                if(!value.compare("neg")) {
                                    value = "0";
                                }
                                break;

                            // uro
                            case 6:
                                isAssigned = true;
                                
                                if(parsed.count() < 3) throw Exception("parse urobil");
                                value = parsed[2];
                                
                                code = "UBG"; units = "MG_DL";      isNumber = true;
                                if(!value.compare("norm")) {
                                    value = "0";
                                }
                                break;

                            // ket
                            case 7:
                                isAssigned = true;
                                
                                if(parsed.count() < 4) throw Exception("parse ket");
                                value = parsed[3];
                                
                                code = "KET"; units = "MG_DL";      isNumber = true;
                                if(!value.compare("neg")) {
                                    value = "0";
                                }
                                break;

                            // prot
                            case 8:
                                isAssigned = true;
                                
                                if(parsed.count() < 3) throw Exception("parse prot");
                                value = parsed[2];
                                
                                code = "BEL"; units = "MG_DL";      isNumber = true;
                                if(!value.compare("neg")) {
                                    value = "0";
                                }
                                break;

                            // nit
                            case 9:
                                isAssigned = true;

                                if(parsed.count() < 3) throw Exception("parse nit");
                                value = parsed[2];
                                
                                code = "NIT"; units = "UNKNOWN";    isNumber = false;
                                if(!value.compare("neg")) {
                                    value = "OTRIC";
                                }
                                else {
                                    value = "POLOJIT";
                                }
                                break;

                            // glu
                            case 10:
                                isAssigned = true;

                                if(parsed.count() < 3) throw Exception("parse glu");
                                value = parsed[2];
                                
                                code = "GLU"; units = "MG_DL";      isNumber = true;
                                if(!value.compare("neg")) {
                                    value = "0";
                                }
                                break;

                            // ph
                            case 11:
                                isAssigned = true;

                                if(parsed.count() < 3) throw Exception("parse ph");
                                value = parsed[2];

                                code = "PH "; units = "UNKNOWN";    isNumber = true;
                                break;

                            // uv
                            case 12:
                                isAssigned = true;

                                if(parsed.count() < 3) throw Exception("parse uv");
                                value = parsed[2];

                                code = "UV "; units = "UNKNOWN";    isNumber = true;
                                if(!value.compare("<=1.005")) {
                                    value = "<1.005";
                                    isNumber = false;
                                }
                                else if(!value.compare(">=1.030")) {
                                    value = ">1.030";
                                    isNumber = false;
                                }
                                break;

                            // leu
                            case 13:
                                isAssigned = true;

                                if(parsed.count() < 3) throw Exception("parse leu");
                                value = parsed[2];

                                code = "LEJ"; units = "LEU_MKL";      isNumber = true;
                                if(!value.compare("neg")) {
                                    value = "0";
                                }
                                break;

                            // ack
                            case 14:
                                isAssigned = true;

                                if(parsed.count() < 4) throw Exception("parse ack");
                                value = parsed[3];

                                code = "ACK"; units = "MG_DL";        isNumber = true;
                                if(!value.compare("neg")) {
                                    value = "0";
                                }
                                break;

                            // color
                            case 15:
                                break;

                            // cla
                            case 16:
                                break;

                            default:
                            break;
                        }

                        break;

                    case URISCANPRO_SKIP_LINE:
                        state = URISCANPRO_RESULTS;
                        break;

                    default:
                        break;
                }

                if (lineIndex > 16) {
                    state = URISCANPRO_END_RESULT;
                }
                else if (lineIndex > 2) {
                    state = URISCANPRO_RESULTS;
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
