
#ifndef MIURA300__H
#define MIURA300__H


#include <string>


#include <Poco/Runnable.h>
#include <Poco/Thread.h>
#include <Poco/Util/AbstractConfiguration.h>


#include "../../analyzer/Analyzer.h"
#include "../../analyzer/AnalyzerDispatcher.h"


#include "../../analyzer/protocols/astm/AstmTransfer.h"


using std::string;
using std::pair;


using Poco::Runnable;
using Poco::Thread;
using Poco::Util::AbstractConfiguration;


//============================================================================//
class Miura300 : public Analyzer
{
public:

    Miura300(Arguments &args);

    virtual ~Miura300();

    void run();

protected:
    
    string onSetConfig(const string &a);
    string onRemoveSample(const string &a);
    
    string sendWorksheet(const string &a);
    string receiveResults(const string &a);

    
    string sendWorksheetTask();
    string receiveResultsTask();
    
    SharedPtr<astm::AstmTransfer>   transfer;

    // make atomic!
    bool                            _isRequestResults, _isSendWorksheet;
};


extern "C"
{
    int POCO_LIBRARY_API registerInstrument(AnalyzerDispatcher &dispatcher);
    int POCO_LIBRARY_API unregisterInstrument(AnalyzerDispatcher &dispatcher);
}

#endif
