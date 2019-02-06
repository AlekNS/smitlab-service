
#include <common/stdafx.h>


#include <Poco/ClassLibrary.h>


#include "../../analyzer/AnalyzerDispatcher.h"
#include "LauraSmart.h"
#include "LauraSmartResultReader.h"



using namespace std;
using namespace Poco;


//============================================================================//
AnalyzerLauraSmart::AnalyzerLauraSmart(Arguments &args) :
    Analyzer(args)
{
    try {

        _storage->begin();

        _storage->setConfig("information.storage.version", ANALYZERS_STORAGE_VERSION_STRING);
        _storage->setConfig("information.details.samples.supplier", "manual");
        _storage->setConfig("settings.places.count",  "1");
        _storage->setConfig("settings.places.default",  "0001");
        _storage->setConfig("settings.places.force_to", "");

        // common
        _storage->setParameter("UV ", "RESEARCH_COMMON_URINE_ANALYSIS", "KAU_FIZ_SVOISTVA",
            "KAU_UDELNIY_VES",  10, 3,
                "if tonumber(value) ~= nil then  "
                "    setValue(value * 1000.0)  "
                "end"
            );

        _storage->setParameter("LEJ", "RESEARCH_COMMON_URINE_ANALYSIS", "KAU_HIM_SVOISTVA",
            "KAU_LEJKOCITY_SCR",    20, 2);

        _storage->setParameter("NIT", "RESEARCH_COMMON_URINE_ANALYSIS", "KAU_HIM_SVOISTVA",
            "KAU_NITRITY",      30, 2);

        _storage->setParameter("rN ", "RESEARCH_COMMON_URINE_ANALYSIS", "KAU_FIZ_SVOISTVA",
            "KAU_PH",           40, 3);

        _storage->setParameter("BEL", "RESEARCH_COMMON_URINE_ANALYSIS", "KAU_HIM_SVOISTVA",
            "KAU_BELOK",        50, 3);

        _storage->setParameter("GL\xDA", "RESEARCH_COMMON_URINE_ANALYSIS", "KAU_HIM_SVOISTVA",
            "KAU_GLUKOZA",      60);

        _storage->setParameter("KET", "RESEARCH_COMMON_URINE_ANALYSIS", "KAU_HIM_SVOISTVA",
            "KAU_KETONI",       70, 3);

        _storage->setParameter("UBG", "RESEARCH_COMMON_URINE_ANALYSIS", "KAU_HIM_SVOISTVA",
            "KAU_UROBILINOGEN", 80, 3);

        _storage->setParameter("BIL", "RESEARCH_COMMON_URINE_ANALYSIS", "KAU_HIM_SVOISTVA",
            "KAU_BILIRUBIN",    90, 3);

        _storage->setParameter("KRV", "RESEARCH_COMMON_URINE_ANALYSIS", "KAU_HIM_SVOISTVA",
            "KAU_ERITROCITY_SCR",   100, 3);


        generatePlaces();


        _storage->commit();

    }
    catch(Exception &e) {
        _storage->rollback();
    }

}

//============================================================================//
AnalyzerLauraSmart::~AnalyzerLauraSmart()
{
}

//============================================================================//
void AnalyzerLauraSmart::run()
{
    Analyzer::run();

    // create reader
    _resultReader = new AnalyzerLauraSmartResultReader(*_io, *_subConfig);

    while(!_isTermination) {

        try {
            processSingleResult();
        }
        catch(Exception &e) {
            poco_error_f1(Logger::get("AnalyzerLauraSmart"),
                "fail analyzer main loop, %s", e.displayText());
            stop();
            return;
        }
        catch(exception &e) {
            poco_error_f1(Logger::get("AnalyzerLauraSmart"),
                "fail analyzer main loop, %s", string(e.what()));
            stop();
            return;
        }

        _thread.sleep(50);
    }

}


//============================================================================//
POCO_BEGIN_MANIFEST(Analyzer)
POCO_END_MANIFEST

void pocoInitializeLibrary()    { }
void pocoUninitializeLibrary()  { }

int registerInstrument(AnalyzerDispatcher &dispatcher)
{
    dispatcher.registerAnalyzerFactory<AnalyzerLauraSmart>("laura_smart");
    return 0;
}

int unregisterInstrument(AnalyzerDispatcher &dispatcher)
{
    return 0;
}
