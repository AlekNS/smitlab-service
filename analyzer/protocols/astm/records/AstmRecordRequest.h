
#ifndef ASTMREQUEST_H
#define	ASTMREQUEST_H

#include "AstmRecord.h"

namespace astm {

class AstmRecordRequest: public AstmRecord {
public:
    enum {
        ID = 'Q'
    };

    const int       fieldsCount;
    
    AstmField       fields[11];
    
    AstmField       &startId;
    AstmField       &endId;
    AstmField       &test;
    AstmField       &timeLimits;
    AstmField       &timeStart;
    AstmField       &timeEnd;
    AstmField       &physName;
    AstmField       &physPhone;
    AstmField       &userField1;
    AstmField       &userField2;
    AstmField       &status;
    
    AstmRecordRequest();
    virtual ~AstmRecordRequest();

    void encode(string &message, const AstmFieldDelimiters &delimiters);
    void decode(const string& message, const AstmFieldDelimiters& delimiters);

    static bool isAllow(const char &previousRecordType);

    static bool isType(const string &frameData);

};

}

#endif
