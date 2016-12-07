
#ifndef ANALYZER_MODULE_H
#define ANALYZER_MODULE_H


#include <map>
#include <string>


#include <Poco/SharedLibrary.h>
#include <Poco/Util/AbstractConfiguration.h>
#include <Poco/Util/Subsystem.h>


#include "AnalyzerDispatcher.h"


using Poco::AutoPtr;
using Poco::SharedLibrary;
using Poco::Util::AbstractConfiguration;
using Poco::Util::Application;
using Poco::Util::Subsystem;


//============================================================================//
class AnalyzerModule : public Subsystem
{
public:
    AnalyzerModule();
    const char* name() const;

protected:

    void initialize(Application& self);
    void uninitialize();

    virtual ~AnalyzerModule();

private:

    map<string, SharedLibrary*>             _sharedLibraries;

};


#endif
