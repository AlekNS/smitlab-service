
#ifndef BECKMAN_CULTURE_ACTD2__H
#define BECKMAN_CULTURE_ACTD2__H


#include <string>


#include <Poco/Runnable.h>
#include <Poco/Thread.h>
#include <Poco/Util/AbstractConfiguration.h>


#include "../../analyzer/Analyzer.h"
#include "../../analyzer/AnalyzerDispatcher.h"


using std::string;
using std::pair;


using Poco::Runnable;
using Poco::Thread;
using Poco::Util::AbstractConfiguration;


//============================================================================//
class AnalyzerBeckmanCultureActD2 : public Analyzer
{
public:

    AnalyzerBeckmanCultureActD2(Arguments &args);

    virtual ~AnalyzerBeckmanCultureActD2();

    void run();

};


extern "C"
{
    int POCO_LIBRARY_API registerInstrument(AnalyzerDispatcher &dispatcher);
    int POCO_LIBRARY_API unregisterInstrument(AnalyzerDispatcher &dispatcher);
}

#endif
