
#ifndef ANALYZER_H
#define ANALYZER_H


#include <map>
#include <string>
#include <fstream>


#include <Poco/Thread.h>
#include <Poco/Runnable.h>
#include <Poco/Util/AbstractConfiguration.h>


namespace common {  class DataEmitter;  }
namespace common {  class IOAdapter;    }

class AnalyzerStorage;
class AnalyzerResultReader;


namespace Poco { 
    namespace Data { class Session; } 
    namespace Net { 
        class StreamSocket;
        class DatagramSocket;

        class SocketStream; 
    } 
}


using std::string;
using std::vector;
using std::map;
using std::pair;
using std::istream;
using std::ostream;


using Poco::FastMutex;
using Poco::AutoPtr;
using Poco::SharedPtr;
using Poco::Runnable;
using Poco::Thread;
using Poco::Util::AbstractConfiguration;
using Poco::Data::Session;


//============================================================================//
class Analyzer : public Runnable
{
public:
    typedef string (Analyzer::*AnalyzerCallback)(const string &argIn);
    
    class Arguments
    {
    public:
        Arguments(string &cd, AbstractConfiguration &conf) :
            code(cd), config(conf)
        {
        }
        string                  &code;
        AbstractConfiguration   &config;
    };

    Analyzer(Arguments &args);
    virtual ~Analyzer();

    virtual void start();
    virtual void stop();

    virtual void run();

    virtual string getCode();
    virtual string getType();
    virtual string getTitle();

    SharedPtr<AnalyzerStorage> storage();

    void getCallbacks(vector<string> &callbacks);
    string invokeCallback(const string &name, const string &arg = "");

    bool checkAllowedConfig(const string &name, const string &value) const;
    
protected:

    map<string, AnalyzerCallback >                      _callbacks;
    typedef map<string, AnalyzerCallback >::iterator    CallbackIterator;

    virtual const string getResultPlaceIdentifier();

    void openCommunication();
    void closeCommunication();

    void processSingleResult();

    void generatePlaces();
    void generatePlaces(const int &fromPlaceIndex, const int &toPlaceIndex);

    // common parameters
    string                              _code;
    Thread                              _thread;
    bool                                _isTermination;

    FastMutex                           _mutex;

    // storage
    SharedPtr<AnalyzerStorage>          _storage;

    // configurations
    AbstractConfiguration               &_config;
    AutoPtr<AbstractConfiguration>      _subConfig;


    // IO
    SharedPtr<common::IOAdapter>        _io;

    // results
    SharedPtr<AnalyzerResultReader>     _resultReader;
    map<string, int>                    _parameterMapper;

    vector<pair<string,string> >        _allowModifyConifgs;

};


#endif
