
#include "AstmRecord.h"


#include <Poco/NumberParser.h>
#include <Poco/NumberFormatter.h>


using namespace astm;

using namespace Poco;

//----------------------------------------------------------------------------//
AstmRecord::AstmRecord() : seq(1), recordType('?') {
}

//----------------------------------------------------------------------------//
AstmRecord::~AstmRecord() {
}

//----------------------------------------------------------------------------//
void AstmRecord::encode(string &message, const AstmFieldDelimiters &delimiters)
{
}

//----------------------------------------------------------------------------//
void AstmRecord::decode(const string &message, const AstmFieldDelimiters &delimiters)
{
}

//----------------------------------------------------------------------------//
void AstmRecord::encodeFields(AstmField *fields, const int &count, 
    string &message, const AstmFieldDelimiters &delimiters)
{
    int emptyIndex = count - 1;
    
    if(seq) {
        message.append(1, delimiters.field);
        message.append(NumberFormatter::format(seq));
    }
    else {
        message.append(1, delimiters.field);
        message.append(1, delimiters.repeat);
        message.append(1, delimiters.component);
        message.append(1, delimiters.escape);
    }
    
    message.append(1, delimiters.field);
    
    // @TODO: Make auto trim or customize fixed count.
    //while(emptyIndex >= 0) {
    //    if(!fields[emptyIndex].isEmpty()) break;
    //    --emptyIndex;
    //}

    for(int i = 0; i <= emptyIndex; i += 1) {
        if(i > 0) {
            message.append(1, delimiters.field);
        }
        message.append(fields[i].getValue(delimiters));
    }
}

//----------------------------------------------------------------------------//
void AstmRecord::decodeFields(AstmField *fields, const int &count,
    const string &message, 
    const AstmFieldDelimiters &delimiters, 
    bool isSequenceExists)
{
    int itemIndex = isSequenceExists ? -2 : -1, found = 0, prev = 0;

    while(true) {
        prev  = found;
        found = message.find(delimiters.field, found);

        if(itemIndex >= 0) {
            fields[itemIndex].setValue(message.substr(
                prev, found < 0 ? (message.size() - prev) : found - prev
            ), delimiters);
        }
        else {
            if(isSequenceExists && itemIndex > -2 && !NumberParser::tryParse(message.substr(
                prev, found < 0 ? (message.size() - prev) : found - prev
            ), seq)) {
                throw AstmFieldFormatWrongTypeIntegerException();
            }
        }
        
        itemIndex += 1;
        
        if(found < 0 || itemIndex >= count)
            break;
        
        found += 1;
    }
}