
#ifndef BECKMAN_CATD2_RESULT_READER
#define BECKMAN_CATD2_RESULT_READER


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
class BeckmanCAD2Reader : public AnalyzerResultReader
{

public:

    BeckmanCAD2Reader(common::IOAdapter &_io);
    virtual ~BeckmanCAD2Reader();

    bool isInstrumentSentResult();
    bool readInstrumentResult(ObjectAnalyzerResultGroup &result,
        map<string, int> &paramsMapper);

protected:

    void sendACK();
    void sendNACK();

    bool checkLineLRC(const string &line);
    bool readAstmLine(string &line);
    bool parseAstmLine(string &line, vector<string> &result);
    void incrementLineIndex();

    char            _delimField,
                    _repeatField,
                    _compField,
                    _escapeField;

    int             _lineIndex;

};


#endif
