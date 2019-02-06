
#ifndef ASTMTRANSFER_H
#define	ASTMTRANSFER_H


#include <istream>
#include <ostream>


#include "../../../common/IOAdapter.h"
#include "AstmRecordFactory.h"


namespace astm {

    
using std::istream;
using std::ostream;
using std::string;
using common::IOAdapter;
   

class AstmTransfer {
public:

    AstmTransfer(IOAdapter &io);
    virtual ~AstmTransfer();

    void writeRecord(AstmRecord &record, AstmFieldDelimiters &delims);
    void receiveFrameData(string &frameData);
    
    void beginRequest();
    void endRequest();

    void beginResponse();
    void endResponse();

private:    

    IOAdapter   &io;
    
    enum {
        STATE_ENQ,
        STATE_ACK,
        STATE_NAK,
        STATE_DATA,
        STATE_TERMINATION,
        STATE_END,
    };
    
    int     _frameNumber, _frameSeek;
    int     _state,  _prevState;
    int     _retries;
    bool    _isDataIntermediate, _isWriting;
    bool    _isDataProcessing;
    
    string  _data;

    void    stateEnq(bool isWriting);
    void    stateAck(bool isWriting);
    void    stateNak(bool isWriting);
    void    stateData(bool isWriting);
    void    stateEnd(bool isWriting);
    void    stateTermination(bool isWriting);
    
    void    processStates();
    
};


}


#endif
