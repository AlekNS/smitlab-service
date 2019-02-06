
#ifndef ASTMHEADER_H
#define	ASTMHEADER_H

#include "AstmRecord.h"

namespace astm {

class AstmRecordHeader: public AstmRecord {
public:
    enum {
        ID = 'H'
    };
    
    AstmFieldDelimiters delimiters;

    const int           fieldsCount;
    
    AstmField           fields[12];

    AstmField           &messageId;
    AstmField           &password;
    AstmField           &sender;
    AstmField           &address;
    AstmField           &reserved;
    AstmField           &phone;
    AstmField           &caps;
    AstmField           &receiver;
    AstmField           &comments;
    AstmField           &processingId;
    AstmField           &version;
    AstmField           &timestamp;
            
    AstmRecordHeader();
    virtual ~AstmRecordHeader();

    void encode(string &message, const AstmFieldDelimiters &delimiters);
    void decode(const string& message, AstmFieldDelimiters& delimiters);

    static bool isAllow(const char &previousRecordType);

    static bool isType(const string &frameData);

};

}

#endif
