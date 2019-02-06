
#include <common/stdafx.h>


#include <Poco/ClassLibrary.h>


#include "../../analyzer/AnalyzerDispatcher.h"
#include "Advia60.h"
#include "Advia60ResultReader.h"



using namespace std;
using namespace Poco;
using namespace common;


//============================================================================//
AnalyzerAdvia60::AnalyzerAdvia60(Arguments &args) :
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
        _storage->setParameter("WBC", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_OBSHCIY_ANALIZ_KROVI",
            "WBC_LEYKOTCITY", 5);

        _storage->setParameter("RBC", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_OBSHCIY_ANALIZ_KROVI",
            "RBC_ERITROTCITY", 10);
        _storage->setParameter("HGB", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_OBSHCIY_ANALIZ_KROVI",
            "HB_GEMOGLOBIN", 20, 1, 
                "if tonumber(value) ~= nil then  "
                "    setValue(value * 10.0); "
                "end"
            );
        _storage->setParameter("HCT", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_OBSHCIY_ANALIZ_KROVI",
            "HT_GEMATOKRIT", 30);
        _storage->setParameter("PLT", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_OBSHCIY_ANALIZ_KROVI",
            "PLT_TROMBOTCITY", 40);

        _storage->setParameter("MCV", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_ERITROTCITARNYE_INDEKSY",
            "MCV_SREDNIY_OBEM_RBC", 50);
        _storage->setParameter("MCH", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_ERITROTCITARNYE_INDEKSY",
            "MCH_SREDNEE_SODERZHANIE_HB_V_RBC", 60);
        _storage->setParameter("MCHC", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_ERITROTCITARNYE_INDEKSY",
            "MCHC_SRE_KON_HB_V_RBC", 70, 1,
                "if tonumber(value) ~= nil then  "
                "    setValue(value * 10.0); "
                "end"
            );

        // ?
        _storage->setParameter("RDW", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_OBSHCIY_ANALIZ_KROVI",
            "RDW_SHIR_RASPR_RBC", 80);
        _storage->setParameter("MPV", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_OBSHCIY_ANALIZ_KROVI",
            "MPV_SRED_OB_PLY", 90);
        _storage->setParameter("PCT", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_OBSHCIY_ANALIZ_KROVI",
            "PCT_TROMBOKRIT", 100);
        _storage->setParameter("PDW", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_OBSHCIY_ANALIZ_KROVI",
            "PDW_OTN_SHIR_PLT_V", 110);

        // formula

        _storage->setParameter("LYM%", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_FORMULA",
            "LYPRC_LIM_OTN_SOD", 120);
        _storage->setParameter("LYM#", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_FORMULA",
            "LYN_LIM_ABS_SOD", 130);

        _storage->setParameter("MON%", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_FORMULA",
            "MOPRC_MON_OTN_SOD", 140);
        _storage->setParameter("MON#", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_FORMULA",
            "MON_MON_ABS_SOD", 150);

        _storage->setParameter("GRA%", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_FORMULA",
            "GRPRC_GRA_OTN_SOD", 160);
        _storage->setParameter("GRA#", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_FORMULA",
            "GRA_GRA_ABS_SOD", 170);

        generatePlaces();

        _storage->commit();

    }
    catch(Exception &e) {
        _storage->rollback();
    }

}

//============================================================================//
AnalyzerAdvia60::~AnalyzerAdvia60()
{
}

//============================================================================//
void AnalyzerAdvia60::run()
{
    Analyzer::run();

    // create reader
    _resultReader = new AnalyzerAdvia60ResultReader(*_io);

    while(!_isTermination) {

        try {
            processSingleResult();
        }
        catch(Exception &e) {
            poco_error_f1(Logger::get("AnalyzerAdvia60"),
                "fail analyzer main loop, %s", e.displayText());
            stop();
            return;
        }
        catch(exception &e) {
            poco_error_f1(Logger::get("AnalyzerAdvia60"),
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
    dispatcher.registerAnalyzerFactory<AnalyzerAdvia60>("advia_60");
    return 0;
}

int unregisterInstrument(AnalyzerDispatcher &dispatcher)
{
    return 0;
}
