
#include "IOAdapter.h"


//#include "analyzer/Analyzer.h"


#include <Poco/Util/AbstractConfiguration.h>
#include <Poco/Net/SocketAddress.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/DatagramSocket.h>
#include <Poco/Net/SocketStream.h>


using namespace std;
using namespace common;
using namespace Poco;
using namespace Poco::Net;


//============================================================================//
IOAdapterSerial::IOAdapterSerial(AbstractConfiguration &config):
IOAdapter(config)
{
    connect();
}

//============================================================================//
void IOAdapterSerial::connect()
{
    if(!_serialPort.isNull()) {
        if(!_serialPort->isOpen()) {
            _serialPort->open();
        }
    }
    else {
        serial::bytesize_t      data_bits;
        serial::parity_t        parity_type;
        serial::stopbits_t      stop_bits;
        serial::flowcontrol_t   flow_mode;

        string data     = _config.getString("communication.serial_data", "8");
        string stop     = _config.getString("communication.serial_stop", "1");
        string parity   = _config.getString("communication.serial_parity", "none");
        string flow     = _config.getString("communication.serial_flow", "none");

        if(!data.compare("8"))      data_bits = serial::eightbits;
        else if(!data.compare("7")) data_bits = serial::sevenbits;
        else if(!data.compare("6")) data_bits = serial::sixbits;
        else if(!data.compare("5")) data_bits = serial::fivebits;
        else { throw Exception("wrong serial data bits"); }

        if(!stop.compare("1"))          stop_bits = serial::stopbits_one;
        else if(!stop.compare("2"))     stop_bits = serial::stopbits_two;
        else if(!stop.compare("1.5"))   stop_bits = serial::stopbits_one_point_five;
        else { throw Exception("wrong serial stop bits"); }

        if(!parity.compare("none"))         parity_type = serial::parity_none;
        else if(!parity.compare("odd"))     parity_type = serial::parity_odd;
        else if(!parity.compare("even"))    parity_type = serial::parity_even;
        else { throw Exception("wrong serial parity type"); }

        if(!flow.compare("none"))             flow_mode = serial::flowcontrol_none;
        else if(!flow.compare("software"))    flow_mode = serial::flowcontrol_software;
        else if(!flow.compare("hardware"))    flow_mode = serial::flowcontrol_hardware;
        else { throw Exception("wrong serial flow mode"); }

        _defaultTimeout = _config.getInt("communication.serial_timeouts", 5000);
            _serialPort = new Serial(
                _config.getString("communication.serial_port"),
                _config.getInt("communication.serial_baud", 9600),
                serial::Timeout::simpleTimeout(
                    _config.getInt("communication.serial_timeouts", _defaultTimeout)
                ),
                data_bits,
                parity_type,
                stop_bits,
                flow_mode);

        if(!_serialPort->isOpen())
            throw IOException();

        input  = new SerialPortInputStream(*_serialPort);
        output = new SerialPortOutputStream(*_serialPort);
    }
}


    
//============================================================================//
void IOAdapterSerial::setReadTimeout(int milliseconds)
{
    if(milliseconds < 1) milliseconds = _defaultTimeout;
    serial::Timeout timeout = serial::Timeout::simpleTimeout(milliseconds);
    _serialPort->setTimeout(timeout);
}


//============================================================================//
void IOAdapterSerial::setWriteTimeout(int milliseconds)
{
}


//============================================================================//
int IOAdapterSerial::isDataBufferAvailable()
{
    return _serialPort->available();
}






//============================================================================//
IOAdapterEthernet::IOAdapterEthernet(AbstractConfiguration& config):
IOAdapter(config)
{
}


//============================================================================//
void IOAdapterEthernet::close()
{
    if(!_tcpSocket.isNull()) {
        if(_tcpSocket->impl()->initialized()) {
            try {
                _tcpSocket->shutdown();
            }
            catch(exception &e) { }
            _tcpSocket->close();
        }
    }
    else if(!_udpSocket.isNull()) {
        _udpSocket->close();
    }
}

//============================================================================//
void IOAdapterEthernet::connect()
{
    bool reconnect = false;
    if(!_tcpSocket.isNull()) {
//        if(!_tcpSocket->impl()->initialized())
            reconnect = true;
    }
    else if(!_udpSocket.isNull()) {
//        _udpSocket->getKeepAlive();
//        if(_udpSocket->impl()->initialized())
            reconnect = true;
    }
    if(_tcpSocket.isNull() && _udpSocket.isNull() || reconnect) {
        string      host = _config.getString("communication.ethernet_host", "");
        const int   port = _config.getInt("communication.ethernet_port", 0);
        const int   connectTimeout = _config.getInt("communication.ethernet_connect_timeout", 5000);
        const int   sendTimeout = _config.getInt("communication.ethernet_send_timeout", 5000);
        const int   recvTimeout = _config.getInt("communication.ethernet_recv_timeout", 5000);

        _isTcp = !_config.getString("communication.ethernet_type", "tcp").compare("tcp");

        _defaultReceiveTimeout = recvTimeout;
        _defaultSendTimeout    = sendTimeout;

        if(_isTcp) {
    //        poco_information_f1(Logger::get(__analyzerLoggerName__),
    //            "use tcp ethernet connection [%s]", getCode());
            if(!_tcpSocket.isNull()) {
                if(_tcpSocket->impl()->initialized()) {
                    try {
                        _tcpSocket->shutdown();
                    }
                    catch(exception &e) { }
                    _tcpSocket->close();
                }
            }

            _tcpSocket = new StreamSocket();
            _tcpSocket->connect(SocketAddress(host, port), Timespan(0, connectTimeout * 1000));
            _tcpSocket->setReuseAddress(true);
            _tcpSocket->setKeepAlive(true);
            _tcpSocket->setSendTimeout(Timespan(0, sendTimeout * 1000));
            _tcpSocket->setReceiveTimeout(Timespan(0, recvTimeout * 1000));

            input  = new SocketInputStream(*_tcpSocket);
            output = new SocketOutputStream(*_tcpSocket);
        }
        else {
    //        poco_information_f1(Logger::get(__analyzerLoggerName__),
    //            "use udp ethernet connection [%s]", getCode());
            if(!_udpSocket.isNull()) {
                _udpSocket->close();
            }

            _udpSocket = new DatagramSocket();
            _udpSocket->connect(SocketAddress(host, port));

            input  = new SocketInputStream(*_udpSocket);
            output = new SocketOutputStream(*_udpSocket);
        }
    }
}


//============================================================================//
int IOAdapterEthernet::isDataBufferAvailable()
{
    if(_isTcp) {
        return _tcpSocket->available();
    }
    return _udpSocket->available();
}


//============================================================================//
void IOAdapterEthernet::setReadTimeout(int milliseconds)
{
    if(milliseconds < 1) milliseconds = _defaultReceiveTimeout;
    if(_isTcp) {
        _tcpSocket->setReceiveTimeout(Timespan(0, milliseconds * 1000));
    }
    else {
        _udpSocket->setReceiveTimeout(Timespan(0, milliseconds * 1000));
    }
}


//============================================================================//
void IOAdapterEthernet::setWriteTimeout(int milliseconds)
{
    if(milliseconds < 1) milliseconds = _defaultSendTimeout;
    if(_isTcp) {
        _tcpSocket->setSendTimeout(Timespan(0, milliseconds * 1000));
    }
    else {
        _udpSocket->setSendTimeout(Timespan(0, milliseconds * 1000));
    }
}
