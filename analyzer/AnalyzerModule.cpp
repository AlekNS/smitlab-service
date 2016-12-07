
#include <common/stdafx.h>


#include <Poco/File.h>
#include <Poco/Util/Application.h>


#include "AnalyzerModule.h"
#include "AnalyzerDispatcher.h"


using namespace std;


using namespace Poco;
using namespace Poco::Util;


//============================================================================//
AnalyzerModule::AnalyzerModule()
{
}

//============================================================================//
const char* AnalyzerModule::name() const
{
    return "AnalyzerModule";
}

//============================================================================//
AnalyzerModule::~AnalyzerModule()
{
}

//============================================================================//
void AnalyzerModule::initialize(Application& self)
{
    poco_information(Logger::get(name()), "initialization");

    vector<string>              files;
    vector<string>::iterator    fiter;

    Path                        pathInstruments(self.config().getString("application.dir"));
    pathInstruments.makeDirectory().pushDirectory("instruments");

    File(pathInstruments).list(files);

    // iterate over all files
    poco_information_f1(Logger::get(name()), "load instruemnts from %s", pathInstruments.toString());
    for(fiter = files.begin(); fiter != files.end(); ++fiter) {
        Path   pathFile(pathInstruments, *fiter);
        File   file(pathFile);
        if(file.isFile() && !string(".").append(pathFile.getExtension()).compare(SharedLibrary::suffix())) {
            poco_information_f1(Logger::get(name()), "try to load instrument %s", *fiter);

            SharedLibrary      *library = NULL;
            try {
                library = new SharedLibrary(pathFile.toString());

                if(!library->isLoaded()) {
                    throw Exception("not loadable");
                }

                if(!library->hasSymbol("registerInstrument") ||
                    !library->hasSymbol("unregisterInstrument")) {
                    throw Exception("not instrument library");
                }

                _sharedLibraries.insert(pair<string, SharedLibrary*>(*fiter, library));

                poco_information(Logger::get(name()), "register instrument...");
                typedef int (*FuncPointer)(AnalyzerDispatcher &dispatcher);

                FuncPointer funcRegisterInstrument = reinterpret_cast<FuncPointer>
                    (library->getSymbol("registerInstrument"));

                funcRegisterInstrument(AnalyzerDispatcher::instance());

            }
            catch(Exception &e) {
                poco_warning_f1(Logger::get(name()),
                    "fail to load shared library, %s", e.displayText());
                if(library) {
                    if(library->isLoaded()) library->unload();
                    delete library;
                }
                continue;
            }
        }
    }

    // create all needed devices
    AnalyzerDispatcher::instance().
        create(Application::instance().config());
}

//============================================================================//
void AnalyzerModule::uninitialize()
{
    poco_information(Logger::get(name()), "shutting down");

    AnalyzerDispatcher::instance().
        release();

    for(map<string, SharedLibrary*>::iterator   library = _sharedLibraries.begin();
        library != _sharedLibraries.end(); ++library)
    {
        library->second->unload();
        delete library->second;
    }
}
