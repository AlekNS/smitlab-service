
#ifndef CLINITEKSTATUS_RESULT_READER_H
#define CLINITEKSTATUS_RESULT_READER_H


#include <string>
#include <vector>
#include <map>
#include <streambuf>
#include <iosfwd>
#include <ios>


#include <Poco/Util/AbstractConfiguration.h>


#include "../../common/CharUtil.h"
#include "../../common/IOAdapter.h"
#include "../../analyzer/AnalyzerStorage.h"
#include "../../analyzer/AnalyzerReader.h"


using std::string;
using std::map;
using std::vector;
using Poco::Util::AbstractConfiguration;


//============================================================================//
class AnalyzerClinitekStatusResultReader : public AnalyzerResultReader
{
public:

    AnalyzerClinitekStatusResultReader(common::IOAdapter &_io, AbstractConfiguration &subConfig);
    virtual ~AnalyzerClinitekStatusResultReader();

    bool isInstrumentSentResult();
    bool readInstrumentResult(ObjectAnalyzerResultGroup &result,
        map<string, int> &paramsMapper);

protected:

    AbstractConfiguration           &_subConfig;
};


#endif
