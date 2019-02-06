
#include "AstmRecordRequest.h"

using namespace astm;

//----------------------------------------------------------------------------//
AstmRecordRequest::AstmRecordRequest() :
    fieldsCount(11),
    startId(fields[0]),
    endId(fields[1]),
    test(fields[2]),
    timeLimits(fields[3]),
    timeStart(fields[4]),
    timeEnd(fields[5]),
    physName(fields[6]),
    physPhone(fields[7]),
    userField1(fields[8]),
    userField2(fields[9]),
    status(fields[10])
{
    recordType = ID;
}

//----------------------------------------------------------------------------//
AstmRecordRequest::~AstmRecordRequest()
{
}

//----------------------------------------------------------------------------//
bool AstmRecordRequest::isAllow(const char &previousRecordType)
{
    switch(previousRecordType) {
        case 'H': case 'Q': return true;
        default:;
    }
    return false;
}

//----------------------------------------------------------------------------//
void AstmRecordRequest::encode(string &message, const AstmFieldDelimiters &delimiters)
{
    encodeFields(fields, fieldsCount, message, delimiters);
}

//----------------------------------------------------------------------------//
void AstmRecordRequest::decode(const string& message, const AstmFieldDelimiters& delimiters)
{
    decodeFields(fields, fieldsCount, message, delimiters);
}

//----------------------------------------------------------------------------//
bool AstmRecordRequest::isType(const string &frameData)
{
    return frameData.size() > 1 && frameData[0] == 'Q';
}
