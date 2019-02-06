
#include <common/stdafx.h>


#include <Poco/Format.h>
#include <Poco/NumberParser.h>
#include <Poco/Thread.h>
#include <Poco/String.h>


#include "Advia60ResultReader.h"


using namespace std;


using namespace Poco;


using namespace common;


//============================================================================//
AnalyzerAdvia60ResultReader::AnalyzerAdvia60ResultReader(common::IOAdapter &_io) :
    AnalyzerResultReader(_io)
{
}

AnalyzerAdvia60ResultReader::~AnalyzerAdvia60ResultReader()
{
}

bool AnalyzerAdvia60ResultReader::isInstrumentSentResult()
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

bool AnalyzerAdvia60ResultReader::readInstrumentResult(ObjectAnalyzerResultGroup &result,
    map<string, int> &paramsMapper)
{

    try {
        _io.input->clear();

        char    receivedData[407] = {0};
        int     receivedCrc  = 0,
                endLineCount = 0;

        _io.input->getline(receivedData, 406, (char)CharUtil::ETX);

        // calculate crc
        for(int i = 0; i < 403; i++) {
            if(receivedData[i] == CharUtil::CR) {
                endLineCount += 1;
            }
            receivedCrc = (receivedCrc ^ receivedData[i]) & 0xFF;
        }

        receivedCrc |= 0x40;

        if(receivedCrc == (int)receivedData[403] && endLineCount == 40) {

            char line[32];
            int  lineIndex = 0;
            string code, value;
            bool   isAssigned;
            istringstream istr;
            istr.str(string(receivedData));

            while(istr.good()) {

                istr.getline(line, 32, (char)CharUtil::CR);

                lineIndex += 1;

                if(line[0] == '-' && line[1] == '-' &&
                    line[2] == '.' && line[3] == '-' && line[4] == '-') {
                    continue;
                }

                isAssigned = false;
                value.clear();
                switch(lineIndex) {
                    case 5:   isAssigned = true; code = "WBC"; break;

                    case 6:   isAssigned = true; code = "LYM#"; break;
                    case 7:   isAssigned = true; code = "LYM%"; break;
                    case 8:   isAssigned = true; code = "MON#"; break;
                    case 9:   isAssigned = true; code = "MON%"; break;
                    case 10:  isAssigned = true; code = "GRA#"; break;
                    case 11:  isAssigned = true; code = "GRA%"; break;

                    case 26:  isAssigned = true; code = "RBC"; break;
                    case 27:  isAssigned = true; code = "HGB"; break;
                    case 28:  isAssigned = true; code = "HCT"; break;
                    case 29:  isAssigned = true; code = "MCV"; break;
                    case 30:  isAssigned = true; code = "MCH"; break;
                    case 31:  isAssigned = true; code = "MCHC"; break;
                    case 32:  isAssigned = true; code = "RDW"; break;

                    case 34:  isAssigned = true; code = "PLT"; break;
                    case 35:  isAssigned = true; code = "MPV"; break;
                    case 36:  isAssigned = true; code = "PCT"; break;
                    case 37:  isAssigned = true; code = "PDW"; break;
                }
                if(isAssigned) {
                    value.append(line, 5);
                    double dblValue;
                    //cout << line << endl;
                    if(NumberParser::tryParseFloat(value, dblValue)) {
                        value.clear();
                        format(value, "%f", dblValue);
                        common::trimRightZeroes(value);

                        map<string,int>::iterator ind = paramsMapper.find(code);
                        ObjectAnalyzerResult      r;

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
                            r.units        = "";
                            r.status       = "ok";
                        }

                        result.results.push_back(r);

                    }
                }

            }

            result.status = "ok";
            if(!result.results.size()) {
                result.comment = "empty";
            }
            
            return true;
        }
        else {
            result.status = "error";
            result.comment = "Frame format error (crc/line count)";
            return false;
        }
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

