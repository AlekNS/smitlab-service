
#ifndef INTERPRETER_HOLDER_H
#define INTERPRETER_HOLDER_H


#include "../lualib/lua.hpp"
#include <string>
#include <vector>


#include "../analyzer/AnalyzerStorage.h"


using std::string;
using std::vector;


class InterpreterHolderException : public Poco::Exception { 
public: InterpreterHolderException(const std::string& msg, const std::string& arg, int code = 0);
};


class InterpreterHolder
{
public:
    InterpreterHolder();
    virtual ~InterpreterHolder();
    void setConfig(vector<RowAnalyzerConfig> &configs);
    void setResultGroup(ObjectAnalyzerResultGroup &group);
    void setExpressionValue(const ObjectAnalyzerResult &result);
    const string& getValue() const;
    const string& getUnit() const;

private:
    static int luaCallBackSetValue(lua_State *state);
    static int luaCallBackSetUnit(lua_State *state);

    lua_State               *lua;
    string                   value, unit;
};

#endif
