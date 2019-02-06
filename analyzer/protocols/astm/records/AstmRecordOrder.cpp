
#include "AstmRecordOrder.h"

using namespace astm;

//----------------------------------------------------------------------------//
AstmRecordOrder::AstmRecordOrder() :
    fieldsCount(30),
    sampleId(fields[0]),
    instrument(fields[1]),
    test(fields[2]),
    priority(fields[3]),
    createdAt(fields[4]),
    sampledAt(fields[5]),
    collectedAt(fields[6]),
    volume(fields[7]),
    collector(fields[8]),
    actionCode(fields[9]),
    dangerCode(fields[10]),
    clinicalInfo(fields[11]),
    deliveredAt(fields[12]),
    biomaterial(fields[13]),
    physician(fields[14]),
    physicianPhone(fields[15]),
    userField1(fields[16]),
    userField2(fields[17]),
    laboratoryField1(fields[18]),
    laboratoryField2(fields[19]),
    modifiedAt(fields[20]),
    instrumentCharge(fields[21]),
    instrumentSection(fields[22]),
    reportType(fields[23]),
    reserved(fields[24]),
    locationWard(fields[25]),
    infectionFlag(fields[26]),
    specimenService(fields[27]),
    laboratory(fields[28]),
    institute(fields[29])
{
    recordType = ID;
}

//----------------------------------------------------------------------------//
AstmRecordOrder::~AstmRecordOrder()
{
}

//----------------------------------------------------------------------------//
bool AstmRecordOrder::isAllow(const char &previousRecordType)
{
    switch(previousRecordType) {
        case 'P': case 'O': case 'R': return true;
        default:;
    }
    return false;
}

//----------------------------------------------------------------------------//
void AstmRecordOrder::encode(string &message, const AstmFieldDelimiters &delimiters)
{
    encodeFields(fields, fieldsCount, message, delimiters);
}

//----------------------------------------------------------------------------//
void AstmRecordOrder::decode(const string& message, const AstmFieldDelimiters& delimiters)
{
    decodeFields(fields, fieldsCount, message, delimiters);
}

//----------------------------------------------------------------------------//
bool AstmRecordOrder::isType(const string &frameData)
{
    return frameData.size() > 1 && frameData[0] == 'O';
}
