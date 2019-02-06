
#ifndef ASTMPATIENT_H
#define	ASTMPATIENT_H

#include "AstmRecord.h"

namespace astm {

class AstmRecordPatient: public AstmRecord {
public:
    enum {
        ID = 'P'
    };
    
    const int           fieldsCount;
    
    AstmField           fields[33];
    
    AstmField           &practiceId;
    AstmField           &laboratoryId;
    AstmField           &id;
    AstmField           &name;
    AstmField           &maidenName;
    AstmField           &birthdate;
    AstmField           &sex;
    AstmField           &race;
    AstmField           &address;
    AstmField           &reserved;
    AstmField           &phone;
    AstmField           &physicianId;
    AstmField           &special1;
    AstmField           &special2;
    AstmField           &height;
    AstmField           &weight;
    AstmField           &diagnosis;
    AstmField           &medication;
    AstmField           &diet;
    AstmField           &practiceField1;
    AstmField           &practiceField2;
    AstmField           &admissionDate;
    AstmField           &admissionStatus;
    AstmField           &location;
    AstmField           &diagnosticCodeNature;
    AstmField           &diagnosticCode;
    AstmField           &religion;
    AstmField           &martialStatus;
    AstmField           &isolationStatus;
    AstmField           &language;
    AstmField           &hospitalService;
    AstmField           &hospitalInstitution;
    AstmField           &dosageCategory;
    
    AstmRecordPatient();
    virtual ~AstmRecordPatient();

    void encode(string &message, const AstmFieldDelimiters &delimiters);
    void decode(const string& message, const AstmFieldDelimiters& delimiters);

    static bool isAllow(const char &previousRecordType);

    static bool isType(const string &frameData);

};

}

#endif
