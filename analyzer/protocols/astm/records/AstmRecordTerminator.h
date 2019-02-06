
#ifndef ASTMTERMINATOR_H
#define	ASTMTERMINATOR_H

#include "AstmRecord.h"

namespace astm {

class AstmRecordTerminator: public AstmRecord {
public:
    enum {
        ID = 'L'
    };

    const int       fieldsCount;
    
    AstmField       fields[1];
    
    AstmField       &code;
    
    AstmRecordTerminator();
    virtual ~AstmRecordTerminator();

    void encode(string &message, const AstmFieldDelimiters &delimiters);
    void decode(const string& message, const AstmFieldDelimiters& delimiters);

    static bool isAllow(const char &previousRecordType);

    static bool isType(const string &frameData);

};

}

#endif
