
#ifndef ASTMRECORDFACTORY_H
#define	ASTMRECORDFACTORY_H

#include "records/AstmRecordHeader.h"
#include "records/AstmRecordPatient.h"
#include "records/AstmRecordOrder.h"
#include "records/AstmRecordTest.h"
#include "records/AstmRecordTerminator.h"
#include "records/AstmRecordRequest.h"


namespace astm {


class AstmRecordFactory {
public:
    AstmRecordFactory();
    virtual ~AstmRecordFactory();
    
    virtual AstmRecord*             record(const char &recordType);

    virtual AstmRecordHeader*       headerRecord();
    virtual AstmRecordPatient*      patientRecord();
    virtual AstmRecordOrder*        orderRecord();
    virtual AstmRecordTest*         testRecord();
    virtual AstmRecordTerminator*   terminatorRecord();
    virtual AstmRecordRequest*      requestRecord();
};


}

#endif
