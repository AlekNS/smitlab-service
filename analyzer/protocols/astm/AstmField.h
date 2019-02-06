
#ifndef ASTMFIELD_H
#define	ASTMFIELD_H


#include <string>
#include <vector>


#include <Poco/LocalDateTime.h>
#include <Poco/Timespan.h>
#include "AstmExceptions.h"


namespace astm {

    
using std::string;
using std::vector;
using Poco::LocalDateTime;
using Poco::Timespan;


/**
 * Astm 1394 delimiters information.
 */
struct AstmFieldDelimiters {
    AstmFieldDelimiters(): 
    field('|'), repeat('\\'), escape('&'), component('^')
    { }
    
    union {
        unsigned char delimiters[4];
        struct {
            unsigned char field;
            unsigned char repeat;
            unsigned char component;
            unsigned char escape;
        };
    };
};




class AstmField {
public:
    
    AstmField();
    virtual ~AstmField();

    
    /**
     * Setup field values (decode field).
     * 
     * @param value Raw field string.
     * @param delimiters Delimiters.
     */
    void setValue(const string &value, const AstmFieldDelimiters &delimiters);
    
    /**
     * Get field values (encode field).
     * 
     * @param delimiters Delimiters.
     * @return Raw field string.
     */
    const string getValue(const AstmFieldDelimiters &delimiters) const;
    
    void setComponentSize(const int &componentSize);
    
    
    // States    
    bool isEmpty() const { return _items.size() == 0; }
    bool isComponent() const { return _items.size() > 1; }
    
    bool isRepeatable() const { return _items.size() > _componentSize; }
    
    int  getComponentSize() const { return _componentSize; }
    int  getRepeatableCount() const { return _items.size() / _componentSize; }

    bool isItemEmpty(const int &index, const int &repeatedIndex = 0);
    
    // For parse
    string asString(const int &index = 0, const int &repeatedIndex = 0);
    int    asInteger(const int &index = 0, const int &repeatedIndex = 0);
    double asFloat(const int &index = 0, const int &repeatedIndex = 0);
    LocalDateTime asDate(const int &index = 0, const int &repeatedIndex = 0);
    LocalDateTime asDatetime(const int &index = 0, const int &repeatedIndex = 0);
    Timespan asTime(const int &index = 0, const int &repeatedIndex = 0);
    
    // For emit
    void setString(const string &val, const int &index = 0, const int &repeatedIndex = 0);
    void setInteger(const int &val, const int &index = 0, const int &repeatedIndex = 0);
    void setFloat(const double &val, const int &index = 0, const int &repeatedIndex = 0);
    void setDate(const LocalDateTime &val, const int &index = 0, const int &repeatedIndex = 0);
    void setDatetime(const LocalDateTime &val, const int &index = 0, const int &repeatedIndex = 0);
    void setTime(const Timespan &val, const int &index = 0, const int &repeatedIndex = 0);
    
private:

    void checkIndex(const int &index, const int &repeatadIndex, bool isIncBuffer);

    int                     _componentSize;
    
    vector<string>          _items;
};

}

#endif
