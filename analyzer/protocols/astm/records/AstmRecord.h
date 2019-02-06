
#ifndef ASTMRECORD_H
#define	ASTMRECORD_H

#include "../AstmField.h"


namespace astm {

    
class AstmRecord {
public:
    
    unsigned char       recordType;
    int                 seq;
    
    AstmRecord();
    virtual ~AstmRecord();
    
    virtual void encode(string &message, const AstmFieldDelimiters &delimiters);
    
    virtual void decode(const string &message, const AstmFieldDelimiters &delimiters);
    
protected:
    
    virtual void encodeFields(AstmField *fields, const int &count, 
        string &message, const AstmFieldDelimiters &delimiters);
    
    virtual void decodeFields(AstmField *fields, const int &count,
        const string &message, const AstmFieldDelimiters &delimiters, bool decodeSequence = true);
    
};


}

#endif
