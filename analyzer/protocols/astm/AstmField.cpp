
#include "AstmField.h"
#include "common/CharUtil.h"
#include <stdio.h>
#include <stdlib.h>


#include <Poco/NumberParser.h>


using namespace astm;

using namespace Poco;


using namespace common;


//----------------------------------------------------------------------------//
AstmField::AstmField() : _componentSize(1)
{
}

//----------------------------------------------------------------------------//
AstmField::~AstmField()
{
}

//----------------------------------------------------------------------------//
void AstmField::setComponentSize(const int &componentSize)
{
    if(_componentSize == 1)
        _items.resize(componentSize);
    _componentSize = componentSize;
}

//----------------------------------------------------------------------------//
void AstmField::setValue(const string &value, const AstmFieldDelimiters &delimiters)
{
    // parse field
    // @TODO: Check received field repeatable components!? Current version skip this feature.
    int itemIndex = 0, repeatCount = 1, repeatIndex = 1, componentIndex = 0,
        componentCount = 1;
    
    for(int i = 0; i < value.size(); i += 1) {
        if(value[i] == delimiters.repeat) {
            repeatCount += 1;
            componentCount = 0;
            if(componentCount > _componentSize)
                _componentSize = componentCount;
        }
        else if(value[i] == delimiters.component) {
            componentCount += 1;
        }
            
    }

    if(componentCount > _componentSize)
        _componentSize = componentCount;
    
    _items.resize(repeatCount * _componentSize);
    
    for(int i = 0; i < value.size(); i += 1) {
        char sym = value[i];
        
        if(sym == delimiters.repeat) {
            itemIndex = repeatIndex * _componentSize;
            componentIndex = 0;
            repeatIndex += 1;
            continue;
        }
        else if(sym == delimiters.component) {
            if(componentIndex >= _componentSize) {
                throw AstmFieldFormatWrongIndexException();
            }
           
            itemIndex += 1;
            componentIndex += 1;
            
            continue;
        }
        else if(sym == delimiters.escape) {
            if(value[i + 1] == 'F') {
                sym = delimiters.field;
            }
            else if(value[i + 1] == 'C') {
                sym = delimiters.component;
            }
            else if(value[i + 1] == 'E') {
                sym = delimiters.escape;
            }
            else if(value[i + 1] == 'R') {
                sym = delimiters.repeat;
            }
            i += 2;
        }
        else if (sym == CharUtil::CR)
            break;
        
        _items[itemIndex].append(1, sym);
    }
}

//----------------------------------------------------------------------------//
const string AstmField::getValue(const AstmFieldDelimiters &delimiters) const
{
    string value;
    bool   isRepeated = false;
    
    for(int repeatIndex = 0, itemIndex = 0; 
        repeatIndex < getRepeatableCount(); repeatIndex += 1) {
        
        if(repeatIndex) value.append(1, delimiters.repeat);
        
        for(int componentIndex = 0; componentIndex < _componentSize; componentIndex += 1) {
            if(componentIndex) value.append(1, delimiters.component);

            if(_items[itemIndex].size()) {
                for(int charIndex = 0; charIndex < _items[itemIndex].size(); charIndex += 1) {
                    
                    const char &sym = _items[itemIndex][charIndex];
                    
                    if(sym == delimiters.component) {
                        value.append(1, delimiters.escape);
                        value.append(1, 'C');
                        value.append(1, delimiters.escape);
                        continue;
                    }
                    else if(sym == delimiters.escape) {
                        value.append(1, delimiters.escape);
                        value.append(1, 'E');
                        value.append(1, delimiters.escape);
                        continue;
                    }
                    else if(sym == delimiters.field) {
                        value.append(1, delimiters.escape);
                        value.append(1, 'F');
                        value.append(1, delimiters.escape);
                        continue;
                    }
                    else if(sym == delimiters.repeat) {
                        value.append(1, delimiters.escape);
                        value.append(1, 'R');
                        value.append(1, delimiters.escape);
                        continue;
                    }
                    
                    value.append(1, sym);
                }
            }
            itemIndex += 1;
            
            isRepeated = false;
        }
    }
    
    return value;
}

//----------------------------------------------------------------------------//
string AstmField::asString(const int &index, const int &repeatedIndex)
{
    checkIndex(index, repeatedIndex, false);
    return _items[index + repeatedIndex * _componentSize];
}

//----------------------------------------------------------------------------//
int    AstmField::asInteger(const int &index, const int &repeatedIndex)
{
    int val;
    
    checkIndex(index, repeatedIndex, false);
    
    if(!NumberParser::tryParse(_items[index + repeatedIndex * _componentSize].c_str(), val)) {
        throw AstmFieldFormatWrongTypeIntegerException();
    }
    
    return val;
}

//----------------------------------------------------------------------------//
double AstmField::asFloat(const int &index, const int &repeatedIndex)
{
    double val;
    
    checkIndex(index, repeatedIndex, false);
    
    if(!NumberParser::tryParseFloat(_items[index + repeatedIndex * _componentSize].c_str(), val)) {
        throw AstmFieldFormatWrongTypeFloatException();
    }
    
    return val;
}

//----------------------------------------------------------------------------//
LocalDateTime AstmField::asDate(const int &index, const int &repeatedIndex)
{
    int y, mn, d;
   
    checkIndex(index, repeatedIndex, false);
    
    const string &item = _items[index + repeatedIndex * _componentSize];
    
    if(item.size() != 8) {
        throw AstmFieldFormatWrongTypeDatetimeException();
    }

    if(!NumberParser::tryParse(item.substr(0, 4), y)) {
        throw AstmFieldFormatWrongTypeDatetimeException();
    }
    if(!NumberParser::tryParse(item.substr(4, 2), mn)) {
        throw AstmFieldFormatWrongTypeDatetimeException();
    }
    if(!NumberParser::tryParse(item.substr(6, 2), d)) {
        throw AstmFieldFormatWrongTypeDatetimeException();
    }
    

    return LocalDateTime(y, mn, d);
}

//----------------------------------------------------------------------------//
LocalDateTime AstmField::asDatetime(const int &index, const int &repeatedIndex)
{
    int y, mn, d, h, m, s;
    
    checkIndex(index, repeatedIndex, false);
    
    const string &item = _items[index + repeatedIndex * _componentSize];
    
    if(item.size() != 14) {
        throw AstmFieldFormatWrongTypeDatetimeException();
    }
    
    if(!NumberParser::tryParse(item.substr(0, 4), y)) {
        throw AstmFieldFormatWrongTypeDatetimeException();
    }
    if(!NumberParser::tryParse(item.substr(4, 2), mn)) {
        throw AstmFieldFormatWrongTypeDatetimeException();
    }
    if(!NumberParser::tryParse(item.substr(6, 2), d)) {
        throw AstmFieldFormatWrongTypeDatetimeException();
    }
    if(!NumberParser::tryParse(item.substr(8, 2), h)) {
        throw AstmFieldFormatWrongTypeDatetimeException();
    }
    if(!NumberParser::tryParse(item.substr(10, 2), m)) {
        throw AstmFieldFormatWrongTypeDatetimeException();
    }
    if(!NumberParser::tryParse(item.substr(12, 2), s)) {
        throw AstmFieldFormatWrongTypeDatetimeException();
    }

    return LocalDateTime(y, mn, d, h, m, s);
}

//----------------------------------------------------------------------------//
Timespan AstmField::asTime(const int &index, const int &repeatedIndex)
{
    int h, m, s;
    
    checkIndex(index, repeatedIndex, false);
    
    const string &item = _items[index + repeatedIndex * _componentSize];
    
    if(item.size() != 6) {
        throw AstmFieldFormatWrongTypeDatetimeException();
    }
    
    if(!NumberParser::tryParse(item.substr(0, 2), h)) {
        throw AstmFieldFormatWrongTypeDatetimeException();
    }
    if(!NumberParser::tryParse(item.substr(2, 2), m)) {
        throw AstmFieldFormatWrongTypeDatetimeException();
    }
    if(!NumberParser::tryParse(item.substr(4, 2), s)) {
        throw AstmFieldFormatWrongTypeDatetimeException();
    }

    return Timespan(0, h, m, s, 0);
}

//----------------------------------------------------------------------------//
bool AstmField::isItemEmpty(const int &index, const int &repeatedIndex)
{
    checkIndex(index, repeatedIndex, false);

    return _items[index + repeatedIndex * _componentSize].size() == 0;
}

//----------------------------------------------------------------------------//
void AstmField::setString(const string &val, const int &index, const int &repeatedIndex)
{
    checkIndex(index, repeatedIndex, true);
    _items[index + repeatedIndex * _componentSize] = val;
}


//----------------------------------------------------------------------------//
void AstmField::setInteger(const int &val, const int &index, const int &repeatedIndex)
{
    char output[18];
    
#if defined(POCO_OS_FAMILY_WINDOWS)
    _snprintf_s(output, 16, "%d", val);
#else
    snprintf(output, 16, "%d", val);
#endif
    
    checkIndex(index, repeatedIndex, true);
    _items[index + repeatedIndex * _componentSize] = output;
}

//----------------------------------------------------------------------------//
void AstmField::setFloat(const double &val, const int &index, const int &repeatedIndex)
{
    char output[18];
    
#if defined(POCO_OS_FAMILY_WINDOWS)
    _snprintf_s(output, 16, "%lf", val);
#else
    snprintf(output, 16, "%lf", val);
#endif
    
    checkIndex(index, repeatedIndex, true);
    _items[index + repeatedIndex * _componentSize] = output;
}

//----------------------------------------------------------------------------//
void AstmField::setDate(const LocalDateTime &val, const int &index, const int &repeatedIndex)
{
    char output[18];
    
#if defined(POCO_OS_FAMILY_WINDOWS)
    _snprintf_s(output, 16, "%04d%02d%02d", val.year(), val.month(), val.day());
#else
    snprintf(output, 16, "%04d%02d%02d", val.year(), val.month(), val.day());
#endif
    
    checkIndex(index, repeatedIndex, true);
    _items[index + repeatedIndex * _componentSize] = output;
}

//----------------------------------------------------------------------------//
void AstmField::setDatetime(const LocalDateTime &val, const int &index, const int &repeatedIndex)
{
    char output[18];
    
#if defined(POCO_OS_FAMILY_WINDOWS)
    _snprintf_s(output, 16, "%04d%02d%02d%02d%02d%02d", 
        val.year(), val.month(), val.day(),
        val.hour(), val.minute(), val.second());
#else
    snprintf(output, 16, "%04d%02d%02d%02d%02d%02d", 
        val.year(), val.month(), val.day(),
        val.hour(), val.minute(), val.second());
#endif
    
    checkIndex(index, repeatedIndex, true);
    _items[index + repeatedIndex * _componentSize] = output;
}
//----------------------------------------------------------------------------//
void AstmField::setTime(const Timespan &val, const int &index, const int &repeatedIndex)
{
    char output[18];
    
#if defined(POCO_OS_FAMILY_WINDOWS)
    _snprintf_s(output, 16, "%02d%02d%02d", 
        val.hours(), val.minutes(), val.seconds());
#else
    snprintf(output, 16, "%02d%02d%02d", 
        val.hours(), val.minutes(), val.seconds());
#endif
    
    checkIndex(index, repeatedIndex, true);
    _items[index + repeatedIndex * _componentSize] = output;
}

//----------------------------------------------------------------------------//
void AstmField::checkIndex(const int &index, const int &repeatadIndex, bool isIncBuffer)
{
    if(index < 0 || index >= _componentSize) 
        throw AstmFieldFormatWrongIndexException();
    if(repeatadIndex >= getRepeatableCount()) {
        if(isIncBuffer)
            _items.resize(repeatadIndex * _componentSize + _componentSize);
        else
            throw AstmFieldFormatWrongIndexException();
    }
}
