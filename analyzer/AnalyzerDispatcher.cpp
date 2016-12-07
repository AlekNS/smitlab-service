
#include <common/stdafx.h>
#include "AnalyzerDispatcher.h"
#include <Poco/Path.h>
#include <Poco/Logger.h>
#include <Poco/Util/Application.h>


#include <iostream>


using namespace std;
using namespace Poco;
using namespace Poco::Util;

using namespace common;

static const char *_loggerName = "AnalyzerDispatcher";


//============================================================================//
AnalyzerDispatcher::AnalyzerDispatcher()
{
    _fabrics = new AbstractFabric<Analyzer, Analyzer::Arguments>();
}

AnalyzerDispatcher::AnalyzerDispatcher(const AnalyzerDispatcher &) { }
AnalyzerDispatcher AnalyzerDispatcher::operator=(const AnalyzerDispatcher &) { return *this; }

//============================================================================//
static AnalyzerDispatcher _dispatcher;

AnalyzerDispatcher& AnalyzerDispatcher::instance()
{
    return _dispatcher;
}


//============================================================================//
AnalyzerDispatcher::~AnalyzerDispatcher()
{
}

//============================================================================//
map<string, SharedPtr<Analyzer> >& AnalyzerDispatcher::getAnalyzers()
{
    return _analyzers;
}

//============================================================================//
bool AnalyzerDispatcher::getAnalyzer(SharedPtr<Analyzer> &analyzer, const string &code)
{
    map<string, SharedPtr<Analyzer> >::iterator iter = _analyzers.find(code);
    if(iter == _analyzers.end()) {
        return false;
    }

    analyzer = iter->second;

    return true;
}

//============================================================================//
typedef map<string, SharedPtr<Analyzer> >::iterator            analyzer_iter;

//============================================================================//
void AnalyzerDispatcher::create(AbstractConfiguration &config)
{
    AbstractConfiguration::Keys             analyzersCodes;
    AbstractConfiguration::Keys::iterator   iter;

    config.keys("analyzers.list", analyzersCodes);

    for(iter = analyzersCodes.begin(); iter != analyzersCodes.end(); ++iter) {

        string type = config.getString("analyzers.list."+(*iter)+".type");

        if(config.has("analyzers.list."+(*iter)+".disabled")) {
            poco_information_f2(Logger::get(_loggerName),
                "analyzer type of [%s] with code [%s] disabled",
                type, string(*iter));
            continue;
        }

        if(_fabrics->isClass(type)) {

            poco_information_f1(Logger::get(_loggerName), "create analyzer for type [%s]", type);

            Analyzer::Arguments  args(*iter, config);
            Analyzer *a = _fabrics->create(type, args);

            poco_information_f2(Logger::get(_loggerName),
                "starting analyzer type of [%s] with code [%s]",
                type, string(a->getCode()));

            try {
                a->start();
                _analyzers.insert(pair<string, SharedPtr<Analyzer> >(*iter, a));
            }
            catch(exception &e) {
                delete a;
            }
        }
        else {
            poco_warning_f1(Logger::get(_loggerName), "unknown analyzer type [%s]... skip this.", type);
        }
    }

    poco_information_f1(Logger::get(_loggerName), "[%d] analyzers was registered", (int)_analyzers.size());
}

//============================================================================//
void AnalyzerDispatcher::release()
{

    poco_information(Logger::get(_loggerName), "release analyzer dispatcher");

    analyzer_iter ai = _analyzers.begin();

    while(ai != _analyzers.end()) {
        poco_information_f1(Logger::get(_loggerName), "stop and delete analyzer with code [%s]", ai->second->getCode());

        ai->second->stop();

        ++ai;
    }

    _analyzers.clear();

    delete _fabrics;

}
