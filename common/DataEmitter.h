
#ifndef DATA_EMITTER_H
#define DATA_EMITTER_H


#include <string>


namespace common {


using std::string;


//============================================================================//
class DataEmitter
{
public:

    virtual ~DataEmitter() { }

    virtual void dict()                        = 0;
    virtual void list()                        = 0;
    virtual void end()                         = 0;

    virtual void key(const string &name)       = 0;

    virtual void item(const string &value)     = 0;
    virtual void item(const int &value)        = 0;
    virtual void item(const double &value)     = 0;

};


}


#endif
