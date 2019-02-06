
#include "AstmRecordTest.h"

using namespace astm;

//----------------------------------------------------------------------------//
AstmRecordTest::AstmRecordTest() :
    fieldsCount(12),
    
    test(fields[0]),
    value(fields[1]),
    units(fields[2]),
    references(fields[3]),
    abnormalFlag(fields[4]),
    abnormalityNature(fields[5]),
    status(fields[6]),
    normsChangedAt(fields[7]),
    operatorId(fields[8]),
    startedAt(fields[9]),
    completedAt(fields[10]),
    instrument(fields[11])   
{
    recordType = ID;
}

//----------------------------------------------------------------------------//
AstmRecordTest::~AstmRecordTest()
{
}

//----------------------------------------------------------------------------//
bool AstmRecordTest::isAllow(const char &previousRecordType)
{
    switch(previousRecordType) {
        case 'R': case 'O': return true;
        default:;
    }
    return false;
}

//----------------------------------------------------------------------------//
void AstmRecordTest::encode(string &message, const AstmFieldDelimiters &delimiters)
{
    encodeFields(fields, fieldsCount, message, delimiters);
}

//----------------------------------------------------------------------------//
void AstmRecordTest::decode(const string& message, const AstmFieldDelimiters& delimiters)
{
    decodeFields(fields, fieldsCount, message, delimiters);
}

//----------------------------------------------------------------------------//
bool AstmRecordTest::isType(const string &frameData)
{
    return frameData.size() > 1 && frameData[0] == 'R';
}
