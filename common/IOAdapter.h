
#ifndef IOADAPTER_H
#define IOADAPTER_H


#include <Poco/SharedPtr.h>
#include <Poco/Util/AbstractConfiguration.h>

#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/DatagramSocket.h>


#include "../serialport/SerialPort.h"


#include <istream>
#include <ostream>


namespace common {


using std::istream;
using std::ostream;
using std::string;


using Poco::SharedPtr;
using Poco::Util::AbstractConfiguration;

using Poco::Net::DatagramSocket;
using Poco::Net::StreamSocket;


//============================================================================//
class IOAdapter
{
public:
    IOAdapter(AbstractConfiguration &config) : _config(config), input(), output() { }
    virtual ~IOAdapter() { }

    virtual void setReadTimeout(int milliseconds)  { }
    virtual void setWriteTimeout(int milliseconds) { }
    virtual int isDataBufferAvailable() { return 0; }

    virtual void connect() { }
    virtual void close()    { }

    SharedPtr<istream>     input;
    SharedPtr<ostream>     output;
    
protected:
    AbstractConfiguration  &_config;
};


//============================================================================//
class IOAdapterSerial: public IOAdapter
{
public:

    IOAdapterSerial(AbstractConfiguration &config);
    ~IOAdapterSerial() { }

    void setReadTimeout(int milliseconds);
    void setWriteTimeout(int milliseconds);
    int  isDataBufferAvailable();

    void connect();
    // void close();
    
protected:

    SharedPtr<Serial>                       _serialPort;
    int                                     _defaultTimeout;
};


//============================================================================//
class IOAdapterEthernet: public IOAdapter
{
public:

    IOAdapterEthernet(AbstractConfiguration &config);
    ~IOAdapterEthernet() { }

    virtual void setReadTimeout(int milliseconds);
    virtual void setWriteTimeout(int milliseconds);
    virtual int  isDataBufferAvailable();

    void connect();
    void close();
    
protected:

    SharedPtr<StreamSocket>      _tcpSocket;
    SharedPtr<DatagramSocket>    _udpSocket;

    bool                         _isTcp;

    int                          _defaultSendTimeout,
                                 _defaultReceiveTimeout;

};


}


#endif
