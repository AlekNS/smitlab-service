
#ifndef ADVIA60_H
#define ADVIA60_H


#include <string>


#include <Poco/Runnable.h>
#include <Poco/Thread.h>
#include <Poco/Util/AbstractConfiguration.h>


#include "../../analyzer/Analyzer.h"


using std::string;
using std::pair;


using Poco::Runnable;
using Poco::Thread;
using Poco::Util::AbstractConfiguration;


//============================================================================//
class AnalyzerAdvia60 : public Analyzer
{
public:

    AnalyzerAdvia60(Arguments &args);

    virtual ~AnalyzerAdvia60();

    void run();

};


extern "C"
{
    int POCO_LIBRARY_API registerInstrument(AnalyzerDispatcher &dispatcher);
    int POCO_LIBRARY_API unregisterInstrument(AnalyzerDispatcher &dispatcher);
}

#endif
