
#ifndef SERIAL_PORT_STREAM_H
#define SERIAL_PORT_STREAM_H


#include <Poco/UnbufferedStreamBuf.h>
#include "serial/serial.h"


using serial::Serial;
using Poco::UnbufferedStreamBuf;


class SerialPortStream : public UnbufferedStreamBuf 
{

public:

    SerialPortStream(Serial &serial);
    virtual ~SerialPortStream();
    
protected:

    int_type readFromDevice();
    int_type writeToDevice(char_type sym);

    Serial &_serial;

};

//============================================================================//
class SerialPortIOS : public virtual std::ios 
{

public:

    SerialPortIOS(Serial &serial);
    virtual ~SerialPortIOS();

    SerialPortStream* rdbuf();
    
protected:

    openmode         _mode;
    SerialPortStream _buf;

};

//============================================================================//
class SerialPortInputStream : public SerialPortIOS, public std::istream 
{

public:

    SerialPortInputStream(Serial &serial);
    virtual ~SerialPortInputStream();

};


//============================================================================//
class SerialPortOutputStream : public SerialPortIOS, public std::ostream 
{

public:

    SerialPortOutputStream(Serial &serial);
    virtual ~SerialPortOutputStream();

};


#endif
