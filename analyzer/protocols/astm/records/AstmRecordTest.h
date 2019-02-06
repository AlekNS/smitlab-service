
#ifndef ASTMTEST_H
#define	ASTMTEST_H

#include "AstmRecord.h"

namespace astm {

class AstmRecordTest: public AstmRecord {
public:
    enum {
        ID = 'R'
    };
    
    const int       fieldsCount;
    
    AstmField       fields[12];
    
    AstmField       &test;
    AstmField       &value;
    AstmField       &units;
    AstmField       &references;
    AstmField       &abnormalFlag;
    AstmField       &abnormalityNature;
    AstmField       &status;
    AstmField       &normsChangedAt;
    AstmField       &operatorId;
    AstmField       &startedAt;
    AstmField       &completedAt;
    AstmField       &instrument;    
    
    AstmRecordTest();
    virtual ~AstmRecordTest();
    
    void encode(string &message, const AstmFieldDelimiters &delimiters);
    void decode(const string& message, const AstmFieldDelimiters& delimiters);
    
    static bool isAllow(const char &previousRecordType);

    static bool isType(const string &frameData);

};

}

#endif
