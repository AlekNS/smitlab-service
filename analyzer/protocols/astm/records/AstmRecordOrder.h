
#ifndef ASTMORDER_H
#define	ASTMORDER_H

#include "AstmRecord.h"

namespace astm {

class AstmRecordOrder: public AstmRecord {
public:
    enum {
        ID = 'O'
    };
    
    const int       fieldsCount;
    
    AstmField       fields[30];
    
    AstmField       &sampleId;
    AstmField       &instrument;
    AstmField       &test;
    AstmField       &priority;
    AstmField       &createdAt;
    AstmField       &sampledAt;
    AstmField       &collectedAt;
    AstmField       &volume;
    AstmField       &collector;
    AstmField       &actionCode;
    AstmField       &dangerCode;
    AstmField       &clinicalInfo;
    AstmField       &deliveredAt;
    AstmField       &biomaterial;
    AstmField       &physician;
    AstmField       &physicianPhone;
    AstmField       &userField1;
    AstmField       &userField2;
    AstmField       &laboratoryField1;
    AstmField       &laboratoryField2;
    AstmField       &modifiedAt;
    AstmField       &instrumentCharge;
    AstmField       &instrumentSection;
    AstmField       &reportType;
    AstmField       &reserved;
    AstmField       &locationWard;
    AstmField       &infectionFlag;
    AstmField       &specimenService;
    AstmField       &laboratory;
    AstmField       &institute;
    
    
    AstmRecordOrder();
    virtual ~AstmRecordOrder();
    
    void encode(string &message, const AstmFieldDelimiters &delimiters);
    void decode(const string& message, const AstmFieldDelimiters& delimiters);

    static bool isAllow(const char &previousRecordType);

    static bool isType(const string &frameData);

};

}

#endif
