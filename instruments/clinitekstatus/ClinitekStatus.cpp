
#include <common/stdafx.h>


#include <Poco/ClassLibrary.h>


#include "../../analyzer/AnalyzerDispatcher.h"
#include "ClinitekStatus.h"
#include "ClinitekStatusResultReader.h"



using namespace std;
using namespace Poco;


//============================================================================//
AnalyzerClinitekStatus::AnalyzerClinitekStatus(Arguments &args) :
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
        _storage->setParameter("SG ", "RESEARCH_COMMON_URINE_ANALYSIS", "KAU_FIZ_SVOISTVA",
            "KAU_UDELNIY_VES",  10, 3,
                "if tonumber(value) ~= nil then  "
                "    setValue(value * 1000.0)  "
                "end"
            );

        _storage->setParameter("LEU", "RESEARCH_COMMON_URINE_ANALYSIS", "KAU_HIM_SVOISTVA",
            "KAU_LEJKOCITY_SCR",    20, 3);

        _storage->setParameter("NIT", "RESEARCH_COMMON_URINE_ANALYSIS", "KAU_HIM_SVOISTVA",
            "KAU_NITRITY",      30, 2);

        _storage->setParameter("pH ", "RESEARCH_COMMON_URINE_ANALYSIS", "KAU_FIZ_SVOISTVA",
            "KAU_PH",           40, 3);

        _storage->setParameter("PRO", "RESEARCH_COMMON_URINE_ANALYSIS", "KAU_HIM_SVOISTVA",
            "KAU_BELOK",        50, 3);

        _storage->setParameter("GLU", "RESEARCH_COMMON_URINE_ANALYSIS", "KAU_HIM_SVOISTVA",
            "KAU_GLUKOZA",      60);

        _storage->setParameter("KET", "RESEARCH_COMMON_URINE_ANALYSIS", "KAU_HIM_SVOISTVA",
            "KAU_KETONI",       70, 3);

        _storage->setParameter("UBG", "RESEARCH_COMMON_URINE_ANALYSIS", "KAU_HIM_SVOISTVA",
            "KAU_UROBILINOGEN", 80, 3);

        _storage->setParameter("BIL", "RESEARCH_COMMON_URINE_ANALYSIS", "KAU_HIM_SVOISTVA",
            "KAU_BILIRUBIN",    90, 3);

        _storage->setParameter("BLD", "RESEARCH_COMMON_URINE_ANALYSIS", "KAU_HIM_SVOISTVA",
            "KAU_ERITROCITY_SCR",   100, 3);

        generatePlaces();

        _storage->commit();

    }
    catch(Exception &e) {
        _storage->rollback();
    }

}

//============================================================================//
AnalyzerClinitekStatus::~AnalyzerClinitekStatus()
{
}

//============================================================================//
void AnalyzerClinitekStatus::run()
{
    Analyzer::run();

    // create reader
    _resultReader = new AnalyzerClinitekStatusResultReader(*_io, *_subConfig);

    while(!_isTermination) {

        try {
            processSingleResult();
        }
        catch(Exception &e) {
            poco_error_f1(Logger::get("AnalyzerClinitekStatus"),
                "fail analyzer main loop, %s", e.displayText());
            stop();
            return;
        }
        catch(exception &e) {
            poco_error_f1(Logger::get("AnalyzerClinitekStatus"),
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
    dispatcher.registerAnalyzerFactory<AnalyzerClinitekStatus>("clinitek_status");
    return 0;
}

int unregisterInstrument(AnalyzerDispatcher &dispatcher)
{
    return 0;
}
