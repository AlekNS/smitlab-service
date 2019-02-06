
#include "AstmRecordTerminator.h"

using namespace astm;

//----------------------------------------------------------------------------//
AstmRecordTerminator::AstmRecordTerminator() :
    fieldsCount(1),
    code(fields[0])
{
    recordType = ID;
}

//----------------------------------------------------------------------------//
AstmRecordTerminator::~AstmRecordTerminator()
{
}

//----------------------------------------------------------------------------//
bool AstmRecordTerminator::isAllow(const char &previousRecordType)
{
    if(previousRecordType == 'L') return false;
    return true;
}

//----------------------------------------------------------------------------//
void AstmRecordTerminator::encode(string &message, const AstmFieldDelimiters &delimiters)
{
    encodeFields(fields, fieldsCount, message, delimiters);
}

//----------------------------------------------------------------------------//
void AstmRecordTerminator::decode(const string& message, const AstmFieldDelimiters& delimiters)
{
    decodeFields(fields, fieldsCount, message, delimiters);
}

//----------------------------------------------------------------------------//
bool AstmRecordTerminator::isType(const string &frameData)
{
    return frameData.size() > 1 && frameData[0] == 'L';
}
