
#ifndef ANALYZER_READER_H
#define ANALYZER_READER_H


#include <fstream>
#include <map>
#include <string>


#include "../common/IOAdapter.h"
#include "../common/StringTools.h"
#include "AnalyzerStorage.h"


using std::istream;
using std::ostream;
using std::map;
using std::string;


//============================================================================//
class AnalyzerResultReader
{
public:

    AnalyzerResultReader(common::IOAdapter &_io) : _io(_io) { }
    virtual ~AnalyzerResultReader() { }

    virtual bool isInstrumentSentResult()       = 0;
    virtual bool readInstrumentResult(ObjectAnalyzerResultGroup &result,
        map<string, int> &paramsMapper)         = 0;

protected:

    common::IOAdapter           &_io;
};


#endif
