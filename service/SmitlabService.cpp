
#include <common/stdafx.h>


#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/Data/SQLite/Connector.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/OptionCallback.h>


#include "SmitlabService.h"
#include "../analyzer/AnalyzerModule.h"
#include "../server/ServerModule.h"


using namespace std;


using namespace Poco;
using namespace Poco::Util;


//============================================================================//
SmitlabService::SmitlabService() :
    _mainLoopping(true)
{
    // display header
    cout << SMITLAB_SERVICE_HEADER_TEXT
        "The service for communication between PC and devices."
        << endl << endl;

    Poco::Data::SQLite::Connector::registerConnector();
}

//============================================================================//
void SmitlabService::prepareAndValidateConfig()
{
    if(!config().has("analyzers.storages.path")) throw Exception(
        "ERROR: analyzers/storages/path not set.");

    Path storagePath(config().getString("analyzers.storages.path"));
    config().setString("analyzers.storages.path", storagePath.makeDirectory().toString());
}

//============================================================================//
void SmitlabService::initialize(Application& self)
{
    if(!_mainLoopping)
        return;

    Path configPath(config().getString("application.dir", ""), "SmitlabService.xml");

    try {

        loadConfiguration(configPath.toString());

        prepareAndValidateConfig();

    }
    catch(Poco::Exception &e) {
        e.rethrow();
    }
    catch(...) {
        throw;
    }

	if(!_mainLoopping)
		return;

    // register all modules
    addSubsystem(new AnalyzerModule());
    addSubsystem(new ServerModule());

    ServerApplication::initialize(self);

    poco_information(logger(), "initialization complete");

}

//============================================================================//
void SmitlabService::uninitialize()
{

    if(!_mainLoopping)
        return;

    poco_information(logger(), "shutting down");
    ServerApplication::uninitialize();
}

//============================================================================//
void SmitlabService::defineOptions(OptionSet& options)
{

    ServerApplication::defineOptions(options);

    options.addOption(Option("help",        "h",
        "display help information for command line arguments"));

    options.addOption(Option("config-file", "f",
        "load configuration data from a file", false, "path", true));

}

//============================================================================//
void SmitlabService::handleOption(const std::string& name,
        const std::string& value)
{

    ServerApplication::handleOption(name, value);

    if(!name.compare("help")) {
        displayHelp();
    }
    else if(!name.compare("config-file")) {
        loadConfiguration(value);
        prepareAndValidateConfig();
    }

}

//============================================================================//
void SmitlabService::displayHelp()
{

    _mainLoopping = false;

    HelpFormatter helpFormatter(options());

    helpFormatter.setIndent(4);
    helpFormatter.setWidth(80);
    helpFormatter.setCommand(commandName());
    helpFormatter.setUsage("{options} ...");
    helpFormatter.setHeader("\noptions:");
    helpFormatter.format(cout);

    stopOptionsProcessing();

}

//============================================================================//
int SmitlabService::main(const std::vector<std::string>& args)
{
    if(!_mainLoopping)
        return Application::EXIT_OK;

    poco_information(logger(), "enter main loop");
    waitForTerminationRequest();
    poco_information(logger(), "leave main loop");

    return Application::EXIT_OK;

}


POCO_SERVER_MAIN(SmitlabService);
