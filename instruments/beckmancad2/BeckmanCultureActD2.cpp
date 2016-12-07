
#include <common/stdafx.h>


#include <Poco/ClassLibrary.h>


#include "BeckmanCultureActD2.h"
#include "BeckmanCAD2ResultReader.h"


using namespace std;


using namespace Poco;


using namespace common;


//============================================================================//
AnalyzerBeckmanCultureActD2::AnalyzerBeckmanCultureActD2(Arguments &args) :
    Analyzer(args)
{
    try {
        _storage->begin();


        _storage->setConfig("information.storage.version", ANALYZERS_STORAGE_VERSION_STRING);
        _storage->setConfig("information.details.samples.supplier", "manual");
        _storage->setConfig("settings.places.count",  "1");
        _storage->setConfig("settings.places.default",  "0001");
        _storage->setConfig("settings.places.force_to", "");


        _storage->setParameter("WBC", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_OBSHCIY_ANALIZ_KROVI",
            "WBC_LEYKOTCITY", 0);
        _storage->setParameter("RBC", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_OBSHCIY_ANALIZ_KROVI",
            "RBC_ERITROTCITY", 10);
        _storage->setParameter("Hgb", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_OBSHCIY_ANALIZ_KROVI",
            "HB_GEMOGLOBIN", 20);
        _storage->setParameter("Hct", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_OBSHCIY_ANALIZ_KROVI",
            "HT_GEMATOKRIT", 30);
        _storage->setParameter("Plt", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_OBSHCIY_ANALIZ_KROVI",
            "PLT_TROMBOTCITY", 40);
        _storage->setParameter("MPV", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_OBSHCIY_ANALIZ_KROVI",
            "MPV_SREDN_OBEM_PLT", 45);
        _storage->setParameter("Pct", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_OBSHCIY_ANALIZ_KROVI",
            "PLT_TROMBOKRIT", 46);
        _storage->setParameter("PDW", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_OBSHCIY_ANALIZ_KROVI",
            "PDW_OTNOS_SHIR_PLT_V_OBEM", 47);



        _storage->setParameter("RDW", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_ERITROTCITARNYE_INDEKSY",
            "RDW_SREDNIY_RASPRED_RBC", 50);

        _storage->setParameter("MCV", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_ERITROTCITARNYE_INDEKSY",
            "MCV_SREDNIY_OBEM_RBC", 60);
        _storage->setParameter("MCH", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_ERITROTCITARNYE_INDEKSY",
            "MCH_SREDNEE_SODERZHANIE_HB_V_RBC", 70);
        _storage->setParameter("MCHC", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_ERITROTCITARNYE_INDEKSY",
            "MCHC_SRE_KON_HB_V_RBC", 80);

        _storage->setParameter("LY%", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_FORMULA",
            "LYPRC_LIM_OTN_SOD", 90);
        _storage->setParameter("LY#", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_FORMULA",
            "LYN_LIM_ABS_SOD", 100);


        _storage->setParameter("MO%", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_FORMULA",
            "MOPRC_MON_OTN_SOD", 110);
        _storage->setParameter("MO#", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_FORMULA",
            "MON_MON_ABS_SOD", 120);

        _storage->setParameter("GR%", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_FORMULA",
            "GRPRC_GRA_OTN_SOD", 130);
        _storage->setParameter("GR#", "RESEARCH_COMMON_BLOOD_ANALYSIS", "KAB_FORMULA",
            "GRA_GRA_ABS_SOD", 140);

        generatePlaces();

        _storage->commit();
    }
    catch(Exception &e) {
        _storage->rollback();
    }
}

//============================================================================//
AnalyzerBeckmanCultureActD2::~AnalyzerBeckmanCultureActD2()
{
}

//============================================================================//
void AnalyzerBeckmanCultureActD2::run()
{
    Analyzer::run();

    // create reader
    _resultReader = new BeckmanCAD2Reader(*_io);

    while(!_isTermination) {

        try {
            processSingleResult();
        }
        catch(Exception &e) {
            poco_error_f1(Logger::get("AnalyzerBeckmanCultureActD2"),
                "fail analyzer main loop, %s", e.displayText());
            stop();
            return;
        }
        catch(exception &e) {
            poco_error_f1(Logger::get("AnalyzerBeckmanCultureActD2"),
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
    dispatcher.registerAnalyzerFactory<AnalyzerBeckmanCultureActD2>("beckman_culture_act_d_2");
    return 0;
}

int unregisterInstrument(AnalyzerDispatcher &dispatcher)
{
    return 0;
}
