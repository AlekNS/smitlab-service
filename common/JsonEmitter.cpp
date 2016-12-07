
#include <common/stdafx.h>


#include "JsonEmitter.h"


using namespace std;
using namespace common;


//============================================================================//
JsonDataEmitter::JsonDataEmitter(ostream &out) :
    _out(out),
    _obj(-1)
{
}

//============================================================================//
JsonDataEmitter::~JsonDataEmitter()
{
}

//============================================================================//
void JsonDataEmitter::dict()
{
    switch(_obj) {
        case OBJ_SEC_E:
        case OBJ_ARR_E:
            _out << ",";
        default: break;
    }
    _prev = _obj;
    _out << "{";
    _obj = OBJ_SEC_B;
    _mode.push_back(_obj);
}

//============================================================================//
void JsonDataEmitter::list()
{
    switch(_obj) {
        case OBJ_SEC_E:
        case OBJ_ARR_E:
            _out << ",";
        default: break;
    }
    _prev = _obj;
    _out << "[";
    _obj = OBJ_ARR_B;
    _mode.push_back(_obj);
}

//============================================================================//
void JsonDataEmitter::end()
{
    if(!_mode.size())
        return;

    _prev = _obj;
    _obj = _mode.back();
	_mode.pop_back();
    if(_obj == OBJ_ARR_B) {
        _out << "]";
        _obj = OBJ_ARR_E;
    }
    if(_obj == OBJ_SEC_B) {
        _out << "}";
        _obj = OBJ_ARR_E;
    }
}

//============================================================================//
void JsonDataEmitter::key(const string &name)
{
    _prev = _obj;
    switch(_obj) {
        case OBJ_SEC_E:
        case OBJ_ARR_E:
        case OBJ_VAL:
            _out << ",";
        default: break;
    }
    _out << "\"" << name << "\":";
    _obj = OBJ_KEY;
}

//============================================================================//
void JsonDataEmitter::item(const string &value)
{
    _prev = _obj;
    switch(_obj) {
        case OBJ_SEC_E:
        case OBJ_ARR_E:
        case OBJ_VAL:
            _out << ",";
        default: break;
    }
    _out << "\"" << value << "\"";
    _obj = OBJ_VAL;
}

//============================================================================//
void JsonDataEmitter::item(const int &value)
{
    _prev = _obj;
    switch(_obj) {
        case OBJ_SEC_E:
        case OBJ_ARR_E:
        case OBJ_VAL:
            _out << ",";
        default: break;
    }
    _out << value;
    _obj = OBJ_VAL;
}

//============================================================================//
void JsonDataEmitter::item(const double &value)
{
    _prev = _obj;
    switch(_obj) {
        case OBJ_SEC_E:
        case OBJ_ARR_E:
        case OBJ_VAL:
            _out << ",";
        default: break;
    }
    _out << value;
    _obj = OBJ_VAL;
}
