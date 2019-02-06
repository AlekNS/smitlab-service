
#include "AstmRecordPatient.h"

using namespace astm;

//----------------------------------------------------------------------------//
AstmRecordPatient::AstmRecordPatient() :
    fieldsCount(33),
    practiceId(fields[0]),
    laboratoryId(fields[1]),
    id(fields[2]),
    name(fields[3]),
    maidenName(fields[4]),
    birthdate(fields[5]),
    sex(fields[6]),
    race(fields[7]),
    address(fields[8]),
    reserved(fields[9]),
    phone(fields[10]),
    physicianId(fields[11]),
    special1(fields[12]),
    special2(fields[13]),
    height(fields[14]),
    weight(fields[15]),
    diagnosis(fields[16]),
    medication(fields[17]),
    diet(fields[18]),
    practiceField1(fields[19]),
    practiceField2(fields[20]),
    admissionDate(fields[21]),
    admissionStatus(fields[22]),
    location(fields[23]),
    diagnosticCodeNature(fields[24]),
    diagnosticCode(fields[25]),
    religion(fields[26]),
    martialStatus(fields[27]),
    isolationStatus(fields[28]),
    language(fields[29]),
    hospitalService(fields[30]),
    hospitalInstitution(fields[31]),
    dosageCategory(fields[32])
{
    recordType = ID;
}

//----------------------------------------------------------------------------//
AstmRecordPatient::~AstmRecordPatient()
{
}

//----------------------------------------------------------------------------//
bool AstmRecordPatient::isAllow(const char &previousRecordType)
{
    switch(previousRecordType) {
        case 'H': case 'P': case 'O': case 'R': return true;
        default:;
    }
    return false;
}

//----------------------------------------------------------------------------//
void AstmRecordPatient::encode(string &message, const AstmFieldDelimiters &delimiters)
{
    encodeFields(fields, fieldsCount, message, delimiters);
}

//----------------------------------------------------------------------------//
void AstmRecordPatient::decode(const string& message, const AstmFieldDelimiters& delimiters)
{
    decodeFields(fields, fieldsCount, message, delimiters);
}

//----------------------------------------------------------------------------//
bool AstmRecordPatient::isType(const string &frameData)
{
    return frameData.size() > 1 && frameData[0] == 'P';
}
