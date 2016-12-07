
#include <common/stdafx.h>
#include "SerialPort.h"

//============================================================================//
SerialPortStream::SerialPortStream(Serial &serial) :
    _serial(serial)
{
}

//============================================================================//
SerialPortStream::~SerialPortStream()
{
}

//============================================================================//
SerialPortStream::int_type SerialPortStream::readFromDevice()
{
    uint8_t sym;
    if(!_serial.read(&sym, 1))
        return -1;
    return (int_type)(sym);
}

//============================================================================//
SerialPortStream::int_type SerialPortStream::writeToDevice(char_type sym)
{
    return _serial.write((uint8_t*)&sym, 1);
}




//============================================================================//
SerialPortIOS::SerialPortIOS(Serial &serial) :
    _buf(serial)
{
    poco_ios_init(&_buf);
}

//============================================================================//
SerialPortIOS::~SerialPortIOS()
{

}

//============================================================================//
SerialPortStream* SerialPortIOS::rdbuf()
{
    return &_buf;
}



//============================================================================//
SerialPortInputStream::SerialPortInputStream(Serial &serial) :
    SerialPortIOS(serial),
    std::istream(&_buf)
{
    _mode = std::ios::in;
}

//============================================================================//
SerialPortInputStream::~SerialPortInputStream()
{

}



//============================================================================//
SerialPortOutputStream::SerialPortOutputStream(Serial &serial) :
    SerialPortIOS(serial),
    std::ostream(&_buf)
{
    _mode = std::ios::out;
}

//============================================================================//
SerialPortOutputStream::~SerialPortOutputStream()
{

}

