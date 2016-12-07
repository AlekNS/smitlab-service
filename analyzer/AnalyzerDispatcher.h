
#ifndef ANALYZER_DISPATCHER_H
#define ANALYZER_DISPATCHER_H


#include <map>


#include <Poco/SharedPtr.h>
#include <Poco/Util/AbstractConfiguration.h>


#include "Analyzer.h"
#include "common/AbstractFabric.h"


using std::map;


using Poco::SharedPtr;
using Poco::Util::AbstractConfiguration;


//============================================================================//
class AnalyzerDispatcher
{
public:

    AnalyzerDispatcher();
    virtual ~AnalyzerDispatcher();

    map<string, SharedPtr<Analyzer> >& getAnalyzers();
    bool getAnalyzer(SharedPtr<Analyzer> &analyzer, const string &code);

    template<class ClassType>
    void registerAnalyzerFactory(const string &type) { _fabrics->registerClass<ClassType>(type); }

    void create(AbstractConfiguration &config);
    void release();

    static AnalyzerDispatcher& instance();

protected:

    AnalyzerDispatcher(const AnalyzerDispatcher &);
    AnalyzerDispatcher operator=(const AnalyzerDispatcher &);

    common::AbstractFabric<Analyzer, Analyzer::Arguments>   *_fabrics;
    map<string, SharedPtr<Analyzer> >                       _analyzers;

};


#endif
