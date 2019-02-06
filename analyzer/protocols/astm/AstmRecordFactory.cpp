
#include "AstmRecordFactory.h"

using namespace astm;

//----------------------------------------------------------------------------//
AstmRecordFactory::AstmRecordFactory() {
}

//----------------------------------------------------------------------------//
AstmRecordFactory::~AstmRecordFactory() {
}

AstmRecord* AstmRecordFactory::record(const char &recordType)
{
    switch(recordType) {
        case AstmRecordHeader::ID:  return headerRecord();
        case AstmRecordPatient::ID: return patientRecord();
        case AstmRecordOrder::ID:   return orderRecord();
        case AstmRecordTest::ID:    return testRecord();
        case AstmRecordRequest::ID: return requestRecord();
        case AstmRecordTerminator::ID: return terminatorRecord();
        default:;
    }
    
    return new AstmRecord();
}

//----------------------------------------------------------------------------//
AstmRecordHeader*    AstmRecordFactory::headerRecord()
{
    return new AstmRecordHeader();
}

//----------------------------------------------------------------------------//
AstmRecordPatient*   AstmRecordFactory::patientRecord()
{
    return new AstmRecordPatient();
}

//----------------------------------------------------------------------------//
AstmRecordOrder*     AstmRecordFactory::orderRecord()
{
    return new AstmRecordOrder();
}

//----------------------------------------------------------------------------//
AstmRecordTest*      AstmRecordFactory::testRecord()
{
    return new AstmRecordTest();
}

//----------------------------------------------------------------------------//
AstmRecordTerminator* AstmRecordFactory::terminatorRecord()
{
    return new AstmRecordTerminator();
}

//----------------------------------------------------------------------------//
AstmRecordRequest*   AstmRecordFactory::requestRecord()
{
    return new AstmRecordRequest();
}
