
#ifndef ADVIA60_RESULT_READER_H
#define ADVIA60_RESULT_READER_H


#include <string>
#include <vector>
#include <map>
#include <streambuf>
#include <iosfwd>
#include <ios>


#include "../../common/CharUtil.h"
#include "../../common/IOAdapter.h"
#include "../../analyzer/AnalyzerStorage.h"
#include "../../analyzer/AnalyzerReader.h"


using std::string;
using std::map;
using std::vector;

//============================================================================//
class AnalyzerAdvia60ResultReader : public AnalyzerResultReader
{
public:

    AnalyzerAdvia60ResultReader(common::IOAdapter &_io);
    virtual ~AnalyzerAdvia60ResultReader();

    bool isInstrumentSentResult();
    bool readInstrumentResult(ObjectAnalyzerResultGroup &result,
        map<string, int> &paramsMapper);

};


#endif
