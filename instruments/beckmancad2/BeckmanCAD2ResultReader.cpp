
#include <common/stdafx.h>


#include <Poco/Thread.h>
#include <Poco/String.h>


#include "BeckmanCAD2ResultReader.h"
#include "../../common/IOAdapter.h"


using namespace std;


using namespace Poco;


using namespace common;


//============================================================================//
BeckmanCAD2Reader::BeckmanCAD2Reader(IOAdapter &_io) :
    AnalyzerResultReader(_io),
    _lineIndex(1)                           // start from 1 (ASTM standard)
{

}

//============================================================================//
BeckmanCAD2Reader::~BeckmanCAD2Reader()
{

}

//============================================================================//
bool BeckmanCAD2Reader::checkLineLRC(const string &line)
{
    int size = line.size();

    // wrong short datas
    if(size < 10) {
        poco_error(Logger::get("AnalyzerBeckmanCultureActD2"),
            "line symbol's count < 10");
        return false;
    }

    // check start symbol, end line symbol, end text or continue (long results)
    if(line[0] != CharUtil::STX || line[size-1] != CharUtil::CR ||
        (line[size-4] != CharUtil::ETX && line[size-4] != CharUtil::ETB)) {
        poco_error_f1(Logger::get("AnalyzerBeckmanCultureActD2"),
            "line astm symbols are not present [%s]", line);
        return false;
    }

    // check index line [0,7]
    if(line[1] != _lineIndex + '0') {
        poco_error_f2(Logger::get("AnalyzerBeckmanCultureActD2"),
            "line astm wrong line index [%s]/[%d]", line, _lineIndex);
        return false;
    }

    // check lrc sum
    int lineLRC = (((
            line[size-3] >= 'A' && line[size-3] <= 'F' ?
                line[size-3] - 'A' + 10 :
                line[size-3] - '0'
        ) << 4) | ((
            line[size-2] >= 'A' && line[size-2] <= 'F' ?
                line[size-2] - 'A' + 10 :
                line[size-2] - '0'
        ) & 0xF)) & 0xFF;
    int calcLRC = 0;
    for(int i = 1; i < size - 3; i++)
        calcLRC = (calcLRC + line[i]) & 0xFF;

    if(lineLRC != calcLRC) {
        poco_error_f1(Logger::get("AnalyzerBeckmanCultureActD2"),
            "line astm crc check failed [%s]", line);
    }

    return lineLRC == calcLRC;
}

//============================================================================//
//
// @BUG: This function doesn't allow resend long results for damaged content.
//  When send long result (such as 2 lines) second line will be resend, but parser
//  look at the first line again if second line will be wrong.
//
bool BeckmanCAD2Reader::readAstmLine(string &line)
{
    char data[258];
    int  size;
    string result;
    try {
        _io.input->clear();
        line.clear();
        do {
            size = 0;
            if(_io.input->getline(data, 256, (char)CharUtil::LF)) {
                result = data;
                size   = result.size();
                if(!checkLineLRC(result))           return false;

                // if this long result (doesn't allow resend!!!)
                if(result[size-4] == CharUtil::ETB) {
                    sendACK();
                    incrementLineIndex();
                }
                line += result;
            }
        } while(size > 0 && line[size-4] == CharUtil::ETB); // continue on long result
        return size > 0;
    }
    catch(exception &e) {
        poco_error(Logger::get("AnalyzerBeckmanCultureActD2"),
            "read astm line exception");
    }
    return false;
}

//============================================================================//
void BeckmanCAD2Reader::sendACK()
{
    char symbol = CharUtil::ACK;
    _io.output->write(&symbol, 1);
    Thread::sleep(40); // @TODO: Redo this hack
}

//============================================================================//
void BeckmanCAD2Reader::sendNACK()
{
    char symbol = CharUtil::NACK;
    Thread::sleep(1000); // @TODO: Check ASTM timeouts
    _io.input->clear();
    _io.output->write(&symbol, 1);
    Thread::sleep(1000); // @TODO: Check ASTM timeouts
}

//============================================================================//
bool BeckmanCAD2Reader::parseAstmLine(string &line, vector<string> &result)
{
    int         i = 1,
                size = line.size();
    bool        isEscapedChar = false;
    string      parsed = "";

    while(i < size) {

        if(isEscapedChar) {
            parsed += line[i];
            ++i;
            if(line[i] != _escapeField)
                return false;
            ++i;
        }

        switch(line[i]) {

            case CharUtil::LF:
                return false;
                break;

            default:
                if(line[i] == _escapeField) {
                    isEscapedChar = true;
                    break;
                }
                if(line[i] == CharUtil::CR ||
                    line[i] == _delimField ||
                    line[i] == _compField ||
                    line[i] == CharUtil::ETX) {

                    result.push_back(parsed);
                    parsed = "";
                    break;
                }

                parsed += line[i];
        }

        ++i;
    }

    return true;
}

//============================================================================//
bool BeckmanCAD2Reader::isInstrumentSentResult()
{
    char symbol = 0;
    try {
        _io.input->clear();
        if(_io.input->read(&symbol, 1)) {
            _lineIndex = 1;
            if(symbol == CharUtil::ENQ) {
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

//============================================================================//
void BeckmanCAD2Reader::incrementLineIndex()
{
    _lineIndex = (_lineIndex + 1) & 7;
}

//============================================================================//
bool BeckmanCAD2Reader::readInstrumentResult(ObjectAnalyzerResultGroup &result,
    map<string, int> &paramsMapper)
{
    enum {
        STATE_HEADER,
        STATE_PATIENT,
        STATE_ORDER,
        STATE_RESULT,
        STATE_RESULT_LONG,
        STATE_TERMINATOR,
        STATE_END,

        STATE_REPEAT,
    };

    char symbol;

    const int maxRepeat = 5;

    int repeatCount = 0,
        state       = STATE_HEADER,
        prevState   = -1;

    bool            isContinued = true, isRepeat = false;
    string          line;
    vector<string>  parsed;

    while(isContinued) {

        try {
            if(state != STATE_REPEAT) {
                sendACK();
            }
            else {
                repeatCount += 1;
                sendNACK();
                state = prevState;
                if(repeatCount > maxRepeat) {
                    isContinued = false;
                    break;
                }
            }

        }
        catch(exception &e) {
            poco_error(Logger::get("AnalyzerBeckmanCultureActD2"),
                "send ACK/NACK exception");
        }

        // @TODO: Should be more OOP
        switch(state) {
            case STATE_HEADER:

                if(!readAstmLine(line) || line[2] != 'H')
                    { prevState = state; state = STATE_REPEAT; break; }
                _delimField  = line[3];
                _repeatField = line[4];
                _compField   = line[5];
                _escapeField = line[6];
                incrementLineIndex();
                state = STATE_PATIENT;

            break;

            case STATE_PATIENT:

                if(!readAstmLine(line) || line[2] != 'P')
                    { prevState = state; state = STATE_REPEAT; break; }
                incrementLineIndex();
                state = STATE_ORDER;

            break;

            case STATE_ORDER:

                if(!readAstmLine(line) || line[2] != 'O')
                    { prevState = state; state = STATE_REPEAT; break; }
                incrementLineIndex();
                state = STATE_RESULT;

            break;

            case STATE_RESULT:

                if(!readAstmLine(line))
                { prevState = state; state = STATE_REPEAT; break; }
                incrementLineIndex();
                if(line[2] == 'R') {
                    parsed.clear();
                    if(!parseAstmLine(line, parsed)) {
                        result.status = "warning";
                        result.comment = "Wrong data received from analyzer.";
                        isContinued = false;
                        break;
                    }

                    ObjectAnalyzerResult r;

                    map<string,int>::iterator ind = paramsMapper.find(parsed[5]);
                    if(paramsMapper.end() == ind) {
                        poco_warning_f1(Logger::get("AnalyzerBeckmanCultureActD2"),
                            "unknown parameter [%s]", line);
                        result.status = "warning";
                        result.comment = "Not all parameters had processed.";
                        r.status = "error";
                        r.comment = "Unknown parameter (";
                        r.comment += parsed[5];
                        r.comment += ")";
                        break;
                    }
                    else {
                        r.parameter_id = ind->second;
                        r.val          = trim(parsed[6]);
                        r.flags        = trim(parsed[7]);
                        r.units        = trim(parsed[8]);
                        r.status       = "ok";
                    }

                    result.results.push_back(r);
                }
                else {
                    if(line[2] != 'L')
                    { prevState = state; state = STATE_REPEAT; break; }
                    state = STATE_END;
                }

            break;

            case STATE_END:

                isContinued = false;
                try {
                    _io.input->read(&symbol, 1);
                }
                catch(exception &e) {
                }

            break;
        }

    }

    if(repeatCount > maxRepeat) {
        result.status = "error";
        result.comment = "The number of attempts exceeded the limit.";
        return false;
    }
    else {
        result.status = "ok";

        if(!result.results.size()) {
            result.comment = "empty";
        }
    }

    return true;
}
