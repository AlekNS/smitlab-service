
#include "AstmTransfer.h"


#include <iostream>


#include <Poco/Thread.h>


#include "AstmExceptions.h"
#include "AstmFrame.h"
#include "common/CharUtil.h"


using namespace std;
using namespace Poco;


using namespace astm;
using namespace common;


//============================================================================//
AstmTransfer::AstmTransfer(IOAdapter &io) : 
io(io), _frameNumber(-1)
{
}

//============================================================================//
AstmTransfer::~AstmTransfer()
{
}

//============================================================================//
void AstmTransfer::writeRecord(AstmRecord &record, AstmFieldDelimiters &delims)
{
    _data.assign(1, record.recordType);
    record.encode(_data, delims);
    
    _state = STATE_DATA;
    _isWriting = true;
    _frameSeek = 0;
    _retries = 6;
    
    processStates();
}

//============================================================================//
void AstmTransfer::receiveFrameData(string &frameData)
{
    _data.clear();
    
    _state = STATE_DATA;
    _isWriting = false;
    _frameSeek = 0;
    _retries = 6;
    
    processStates();
    
    frameData.assign(_data);
}

//============================================================================//
void AstmTransfer::beginRequest()
{
    _state = STATE_ENQ;
    _frameNumber = 1;
    _isWriting = true;
    _isDataProcessing = false;
    _retries = 6;
    
    io.connect();

    processStates();
}

//============================================================================//
void AstmTransfer::endRequest()
{
    _state = STATE_END;
    _isWriting = true;
    
    processStates();
    
    io.close();
}

//============================================================================//
void AstmTransfer::beginResponse()
{
    _state = STATE_ENQ;
    _frameNumber = 1;
    _isWriting = false;
    _isDataProcessing = false;
    _retries = 6;

    io.connect();

    processStates();
}

//============================================================================//
void AstmTransfer::endResponse()
{
    _state = STATE_END;
    _isWriting = false;
    
    processStates();
    
    io.close();
}

//============================================================================//
void AstmTransfer::processStates()
{
    bool _processStatesLoop = true;
    while(_processStatesLoop) {
        switch(_state) {
            
            case STATE_ENQ: {
                stateEnq(_isWriting);
            } break;
            
            case STATE_ACK: {
                stateAck(_isWriting);
                if(_prevState == _state) {
                    _processStatesLoop = false;
                }
            } break;
            
            case STATE_NAK: {
                stateNak(_isWriting);
            } break;
            
            case STATE_DATA: { 
                stateData(_isWriting);
            } break;
            
            case STATE_TERMINATION: {
                stateTermination(_isWriting);
                if(_prevState == _state) {
                    _processStatesLoop = false;
                }
            } break;
            
            case STATE_END: {
                stateEnd(_isWriting);
                if(_prevState == _state) {
                    _processStatesLoop = false;
                }
            } break;
        }
        
        _prevState = _state;
    }
}

//============================================================================//
void AstmTransfer::stateEnq(bool isWriting)
{
    _isDataProcessing = false;
    _isDataIntermediate = false;
    
    if(isWriting) {
#ifdef _ASTM_DEBUG
        cout << "Send [ENQ]" << endl;
#endif
        
        io.output->put(CharUtil::ENQ);
        io.output->flush();
        _isWriting = false;
        _state = STATE_ACK;
    }
    else {
        try {
#ifdef _ASTM_DEBUG
            cout << "Receive [ENQ]" << endl;
#endif
            int sym = io.input->get();
            
            switch(sym) {
                case CharUtil::ENQ:
#ifdef _ASTM_DEBUG
            cout << "Receive [ENQ] - OK" << endl;
#endif
                    _isWriting = true;
                    _state = STATE_ACK;
                    break;
                default:
#ifdef _ASTM_DEBUG
            cout << "Received not [ENQ]!!! [" << sym << "]" << endl;
#endif
                    _isWriting = true;
                    _state = STATE_TERMINATION;
            }
        }
        catch(exception &e) {
            throw AstmStateExceptionTimeout("Timeout, no [ENQ] received");
        }
    }
}

//============================================================================//
void AstmTransfer::stateAck(bool isWriting)
{
    if(isWriting) {
#ifdef _ASTM_DEBUG
            cout << "Receive [ACK]" << endl;
#endif
        io.output->put(CharUtil::ACK);
        io.output->flush();
        
        if(_isDataIntermediate) {
            _state = STATE_DATA;
            _isWriting = false;
        }
    }
    else {
        io.setReadTimeout(15000);
        try {
            int sym = io.input->get();
#ifdef _ASTM_DEBUG
            cout << "Receive [ACK]" << endl;
#endif
            switch(sym) {
                case CharUtil::ACK:
#ifdef _ASTM_DEBUG
            cout << "Receive [ACK] - OK" << endl;
#endif
                    if(_isDataIntermediate) {
                        _frameSeek += AstmFrame::MAX_FRAME_SIZE;
                        _state = STATE_DATA;
                        _isWriting = true;
                    }
                    if(_isDataProcessing) {
                        _frameNumber += 1;
                    }
                    break;
                case CharUtil::NACK:
#ifdef _ASTM_DEBUG
            cout << "ERROR: Receive [NACK]!!!" << endl;
#endif
                    _isWriting = true;
                    _retries = 6;
                    _state = STATE_NAK;
                    break;
                case CharUtil::ENQ:
#ifdef _ASTM_DEBUG
            cout << "ERROR: Received [ENQ]!!!" << endl;
#endif
                    _isWriting = true;
                    _state = STATE_ENQ;
                    Thread::sleep(1000);
                    break;
                default:
#ifdef _ASTM_DEBUG
            cout << "ERROR: Receive not [ACK]!!![" << sym << "]" << endl;
#endif
                    _isWriting = true;
                    _state = STATE_TERMINATION;
                    break;
                    
            }
        }
        catch(exception &e) {
            throw AstmStateExceptionTimeout("Timeout, no [ACK]/[NAK]/[ENQ] received");
        }
    }
}

//============================================================================//
void AstmTransfer::stateNak(bool isWriting)
{
    _retries -= 1;

    if(isWriting) {
#ifdef _ASTM_DEBUG
            cout << "Send [NACK]" << endl;
#endif
        io.output->put(CharUtil::NACK);
        io.output->flush();
        Thread::sleep(500);
    }
    else {
#ifdef _ASTM_DEBUG
            cout << "Wait after NACK 10 seconds..." << endl;
#endif
        Thread::sleep(10000);
    }

    if(_retries < 1) {
        _isWriting = true;
        _state = STATE_TERMINATION;
#ifdef _ASTM_DEBUG
            cout << "ERROR: Retries > 6... go to termination" << endl;
#endif
    }
    else {
        if(_isDataProcessing) {
            _isWriting = !isWriting;
            _state = STATE_DATA;
        }
        else {
            _isWriting = !isWriting;
            _state = STATE_ENQ;
        }
    }
}

//============================================================================//
void AstmTransfer::stateEnd(bool isWriting)
{
    _prevState = _state;
    if(isWriting) {
#ifdef _ASTM_DEBUG
            cout << "Send [EOT]" << endl;
#endif
        io.output->put(CharUtil::EOT);
        io.output->flush();
    }
    else {
        io.setReadTimeout(1000);
        try {
#ifdef _ASTM_DEBUG
            cout << "Receive [EOT]" << endl;
#endif
            switch(io.input->get()) {
                case CharUtil::EOT:
#ifdef _ASTM_DEBUG
            cout << "Receive [EOT] - OK" << endl;
#endif
                    break;
                default: 
                    break;
            }
        }
        catch(exception &e) {
#ifdef _ASTM_DEBUG
            cout << "ERROR: Receive no [EOT]" << e.what() << endl;
#endif
        }
    }
}

//============================================================================//
void AstmTransfer::stateTermination(bool isWriting)
{
    _prevState = _state;
#ifdef _ASTM_DEBUG
            cout << "State termination..." << endl;
#endif
    
    if(isWriting) {
#ifdef _ASTM_DEBUG
            cout << "Send [EOT] TERMINATION" << endl;
#endif
        io.output->put(CharUtil::EOT);
        io.output->flush();
    }
    
    throw AstmStateExceptionTermination();
}

//============================================================================//
void AstmTransfer::stateData(bool isWriting)
{
    _isDataProcessing = true;
    if(isWriting) {
        int frameSeek = AstmFrame::write(_frameSeek, _frameNumber, _data, *io.output);

#ifdef _ASTM_DEBUG        
        cout << "WRITE Frame:" << endl;
        cout << _data << endl;
#endif
        
        _isDataIntermediate = frameSeek == _frameSeek;
        _isWriting = false;
        _state = STATE_ACK;
    }
    else {
        _isWriting = true;
        try {
            _isDataIntermediate = AstmFrame::read(_frameNumber, _data, *io.input);
            
#ifdef _ASTM_DEBUG        
        cout << "READ Frame:" << endl;
        cout << _data << endl;
#endif
        
            _state = STATE_ACK;
        }
        catch(AstmFrameException &e) {
            _state = STATE_NAK;
            // @TODO: Log
        }
        catch(exception &e) {
            _state = STATE_NAK;
        }
    }
}
