#ifndef SMITLAB_SERVICE_H
#define SMITLAB_SERVICE_H


#include <Poco/Util/ServerApplication.h>


#ifndef SMITLAB_SERVICE_VERSION_STRING
#define SMITLAB_SERVICE_VERSION_STRING "0.2.3"


#define SMITLAB_SERVICE_HEADER_TEXT \
        "SmitLab Service. Copyright (c) Alexander Sedelnikov. Version " SMITLAB_SERVICE_VERSION_STRING "\n" \
        "Organize workflow process with analyzers, scanners, etc. hardwares.\n" \
        "All rights reserved. "  __DATE__ ".\n"

#endif


using Poco::Logger;
using Poco::Util::Application;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;


class SmitlabService : public ServerApplication
{
public:

    SmitlabService();

private:

    int  main(const std::vector<std::string>& args);

    void initialize(Application& self);
    void uninitialize();

    void defineOptions(OptionSet& options);
    void handleOption(const std::string& name, const std::string& value);

    void displayHelp();

    void prepareAndValidateConfig();

    bool                      _mainLoopping;

};


#endif
