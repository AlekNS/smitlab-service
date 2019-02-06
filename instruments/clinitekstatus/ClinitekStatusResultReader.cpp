
#include <common/stdafx.h>


#include <Poco/Format.h>
#include <Poco/NumberParser.h>
#include <Poco/StringTokenizer.h>
#include <Poco/Thread.h>
#include <Poco/String.h>


#include "ClinitekStatusResultReader.h"


using namespace std;


using namespace Poco;


using namespace common;


//============================================================================//
AnalyzerClinitekStatusResultReader::AnalyzerClinitekStatusResultReader(common::IOAdapter &_io, 
    AbstractConfiguration &subConfig) :
    AnalyzerResultReader(_io),
    _subConfig(subConfig)
{
}

AnalyzerClinitekStatusResultReader::~AnalyzerClinitekStatusResultReader()
{
}

bool AnalyzerClinitekStatusResultReader::isInstrumentSentResult()
{
    try {
        char symbol = 0;
        _io.input->clear();
        if(_io.input->read(&symbol, 1)) {
            if(symbol >= '0' && symbol <= '9') {
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


bool AnalyzerClinitekStatusResultReader::readInstrumentResult(ObjectAnalyzerResultGroup &result,
    map<string, int> &paramsMapper)
{

    try {
        int              receivedDataSize   = 2048;
        char             receivedData[2048] = {0};

        _io.input->clear();

        _io.input->getline(receivedData, receivedDataSize - 2, (char)CharUtil::CR);
        
        int receivedFromStreamSize = 0;
        for(receivedFromStreamSize = 0;
            receivedFromStreamSize < receivedDataSize - 2 &&
                receivedData[receivedFromStreamSize] != CharUtil::CR;
            ++receivedFromStreamSize);

        receivedData[receivedFromStreamSize] = 0;

        StringTokenizer parsedItems(receivedData, ",");
        if(parsedItems.count() != 41) {
            poco_error(Logger::get("AnalyzerClinitekStatus"),
                "fail to parse received data, the parsed items count not equals to 41 count");
            return false;
        }

        string  code, value, units, comment;
        bool    isAssigned, isNumber, isMeasuredInSi = !_subConfig.getString("units.type", "si").compare("si");
        int     itemIndex = 0;

        StringTokenizer::Iterator item = parsedItems.begin();
        istringstream istr; 
        while(item != parsedItems.end()) {

            isAssigned = false;
            isNumber   = false;
            code.clear();
            value.clear();
            units.clear();

            switch(itemIndex) {
            
                // GLU
                case 12:
                    isAssigned = true; code = *item; 
                    if(isMeasuredInSi) units = "MMOL_L";     else units = "MG_DL";
                    ++item; ++itemIndex;

                    value = StringTokenizer(item->c_str(), " ")[0];
                    isNumber = true; 

                    if(!item->find("Negative"))         { value = "0";                          }
                    else if(!item->find(">=55"))        { value = ">55";    isNumber = false;   }
                    else if(!item->find(">=1000"))      { value = ">1000";  isNumber = false;   }

                    break;

                // BIL
                case 15:
                    isAssigned = true; code = *item;
                    ++item; ++itemIndex;

                    units = "UNKNOWN";

                    value = StringTokenizer(item->c_str(), " ")[0];

                    if(!item->find("Negative"))      { value = "OTRIC";     }
                    else if(!item->find("Small"))    { value = "MALOE";     }
                    else if(!item->find("Moderate")) { value = "SREDNEE";   }
                    else if(!item->find("Large"))    { value = "BOLSHOE";   }

                    break;

                // KET
                case 18:
                    isAssigned = true; code = *item; 
                    if(isMeasuredInSi) units = "MMOL_L";     else units = "MG_DL";
                    ++item; ++itemIndex;
                    isNumber = true; 

                    value = StringTokenizer(item->c_str(), " ")[0];

                    if(!item->find("Negative"))      { value = "0"; }
                    else if(!item->find("Trace"))    { value = "<0.5";  isNumber = false; }  // TRACE!? < 4
                    else if(!item->find(">=15.6"))   { value = ">15.6"; isNumber = false; }
                    else if(!item->find(">=160"))    { value = ">160";  isNumber = false; }

                    break;

                // SG
                case 21:
                    isAssigned = true; code = *item; 
                    ++item; ++itemIndex;
                    isNumber = true; 

                    units = "UNKNOWN";

                    value = StringTokenizer(item->c_str(), " ")[0];

                    if(!item->find("<=1.005"))         { value = "<1005"; isNumber = false; }
                    else if(!item->find(">=1.030"))    { value = ">1030"; isNumber = false; }

                    break;

                // BLO/BLD
                case 24:
                    isAssigned = true; code = *item; 
                    code = "BLD";
                    if(isMeasuredInSi) units = "ERY_MKL"; else units = "UNKNOWN";
                    ++item; ++itemIndex;

                    value = StringTokenizer(item->c_str(), " ")[0];

                    if(!item->find("Negative"))              { value = "OTRIC";                     }
                    else if(!item->find("Trace-lysed"))      { value = "SLED_LYSED";                }
                    else if(!item->find("Trace-intact"))     { value = "SLED_INTACT";               }
                    else if(!item->find("Small"))            { value = "MALOE";                     }
                    else if(!item->find("Moderate"))         { value = "SREDNEE";                   }
                    else if(!item->find("Large"))            { value = "BOLSHOE";                   }
                    // only for si
                    else if(!item->find("Ca 25"))            { value = "25";     }
                    else if(!item->find("Ca 80"))            { value = "80";     }
                    else if(!item->find("Ca 200"))           { value = "200";    }

                    break;

                // PH
                case 27:
                    isAssigned = true; code = *item;
                    ++item; ++itemIndex;
                    isNumber = true;
                    
                    units = "UNKNOWN";

                    value = StringTokenizer(item->c_str(), " ")[0];

                    if(!item->find(">=9.0"))         { value = ">9"; isNumber = false; }

                    break;

                // PRO
                case 30:
                    isAssigned = true; code = *item; 
                    if(isMeasuredInSi) units = "G_L";       else units = "MG_DL";
                    ++item; ++itemIndex;
                    isNumber = true; 

                    value = StringTokenizer(item->c_str(), " ")[0];
                    
                    if(!item->find("Negative"))         { value = "0";                          }
                    else if(!item->find("Trace"))       { value = "<1";       isNumber = false; }
                    else if(!item->find(">=300"))       { value = ">300";     isNumber = false; }
                    else if(!item->find(">=3.0"))       { value = ">3";       isNumber = false; }

                    break;

                // URO/UBG
                case 33:
                    isAssigned = true; code = *item; 
                    code = "UBG";
                    if(isMeasuredInSi) units = "MMOL_L";     else units = "MG_DL";
                    ++item; ++itemIndex;
                    isNumber = true; 

                    value = StringTokenizer(item->c_str(), " ")[0];

                    if(!item->find(">+131"))        { value = ">131";   isNumber = false; }
                    else if(!item->find(">=131"))   { value = ">131";   isNumber = false; }
                    else if(!item->find(">=8.0"))   { value = ">8";     isNumber = false; }

                    break;

                // NIT
                case 36:
                    isAssigned = true; code = *item; 
                    ++item; ++itemIndex;

                    units = "UNKNOWN";

                    if(!item->find("Negative"))      { value = "OTRIC";   }
                    else if(!item->find("Positive")) { value = "POLOJIT";     }

                    break;

                // LEU
                case 39:
                    isAssigned = true; code = *item; 
                    if(isMeasuredInSi) units = "LEU_MKL"; else units = "UNKNOWN";
                    ++item; ++itemIndex;

                    value = StringTokenizer(item->c_str(), " ")[0];

                    if(!item->find("Negative"))              { value = "OTRIC";                   }
                    else if(!item->find("Trace"))            { value = "SLED";                    }
                    else if(!item->find("Small"))            { value = "MALOE";                   }
                    else if(!item->find("Moderate"))         { value = "SREDNEE";                 }
                    else if(!item->find("Large"))            { value = "BOLSHOE";                 }
                    // only for si
                    else if(!item->find("Ca 15"))            { value = "15";                    }
                    else if(!item->find("Ca 70"))            { value = "70";                    }
                    else if(!item->find("Ca 125"))           { value = "125";                   }
                    else if(!item->find("Ca 500"))           { value = "500";                   }

                    break;
                    
                default:
                    ++item;
                    ++itemIndex;
            }

            if(isAssigned) {
                bool isNumberParsed = false;
                double dblValue = 0;

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
                    r.comment      = comment;
                    r.flags        = "";
                    r.units        = trim(units);
                    r.status       = "ok";
                }

                result.results.push_back(r);
            }
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
