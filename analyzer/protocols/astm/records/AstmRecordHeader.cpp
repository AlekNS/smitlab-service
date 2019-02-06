
#include "AstmRecordHeader.h"

using namespace astm;

//----------------------------------------------------------------------------//
AstmRecordHeader::AstmRecordHeader() : 
    fieldsCount(12),
    messageId(fields[0]),
    password(fields[1]),
    sender(fields[2]),
    address(fields[3]),
    reserved(fields[4]),
    phone(fields[5]),
    caps(fields[6]),
    receiver(fields[7]),
    comments(fields[8]),
    processingId(fields[9]),
    version(fields[10]),
    timestamp(fields[11])
{
    recordType = ID;
    seq = 0;
}


//----------------------------------------------------------------------------//
AstmRecordHeader::~AstmRecordHeader()
{
}

//----------------------------------------------------------------------------//
bool AstmRecordHeader::isAllow(const char &previousRecordType)
{
    if(previousRecordType == 'L')
        return true;
    return false;
}

//----------------------------------------------------------------------------//
void AstmRecordHeader::encode(string &message, const AstmFieldDelimiters &delimiters)
{
    encodeFields(fields, fieldsCount, message, delimiters);
}

//----------------------------------------------------------------------------//
void AstmRecordHeader::decode(const string& message, AstmFieldDelimiters& delimiters)
{
    delimiters.field = message[1];
    delimiters.repeat = message[2];
    delimiters.component = message[3];
    delimiters.escape = message[4];
    
    decodeFields(fields, fieldsCount, message.substr(5), delimiters, false);
}

//----------------------------------------------------------------------------//
bool AstmRecordHeader::isType(const string &frameData)
{
    return frameData.size() > 1 && frameData[0] == 'H';
}
