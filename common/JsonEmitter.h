
#ifndef JSON_DATA_EMITTER_H
#define	JSON_DATA_EMITTER_H


#include <vector>
#include <ostream>


#include "DataEmitter.h"


namespace common {


using std::vector;
using std::ostream;


//============================================================================//
class JsonDataEmitter : public DataEmitter
{
public:

    JsonDataEmitter(ostream &out);
    virtual ~JsonDataEmitter();

    void dict();
    void list();
    void end();

    void key(const string &name);

    void item(const string &value);
    void item(const int &value);
    void item(const double &value);

private:

    enum        { OBJ_SEC_B, OBJ_SEC_E,
                  OBJ_ARR_B, OBJ_ARR_E,
                  OBJ_KEY,
                  OBJ_VAL };

    vector<int>         _mode;

    int                 _obj, _prev;

    ostream            &_out;

};


}

#endif
