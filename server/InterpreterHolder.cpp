
#include <common/stdafx.h>


#include "InterpreterHolder.h"
#include "AnalyzerRequests.h"


using namespace std;


using namespace Poco;


InterpreterHolderException::InterpreterHolderException(const std::string& msg, const std::string& arg, int code) :
    Exception(msg, arg, code)
{
}



InterpreterHolder::InterpreterHolder() : lua(luaL_newstate())
{
    luaopen_base(lua);
    luaopen_math(lua);
    luaopen_string(lua);
    lua_pushlightuserdata(lua, (void*)this);
    lua_setglobal(lua, "_THIS");
    lua_register(lua, "setValue", &InterpreterHolder::luaCallBackSetValue);
    lua_register(lua, "setUnits",  &InterpreterHolder::luaCallBackSetUnit);
//    lua_register(lua, "setFlags", &InterpreterHolder::luaCallBackSetFlags);
//    lua_register(lua, "setStatus",&InterpreterHolder::luaCallBackSetStatus);
}

InterpreterHolder::~InterpreterHolder()
{
    lua_close(lua);
}

void InterpreterHolder::setConfig(vector<RowAnalyzerConfig> &configs)
{
    RowAnalyzerConfig::iter   confIter, confEnd;
    confIter = configs.begin();
    confEnd  = configs.end();

    lua_newtable(lua);
    while(confIter != confEnd) {
        lua_pushstring(lua, confIter->code.c_str());
        lua_pushstring(lua, confIter->value.c_str());
        lua_settable(lua, -3);
        ++confIter;
    }
    lua_setglobal(lua, "configs");
}

void InterpreterHolder::setResultGroup(ObjectAnalyzerResultGroup &group)
{
    ObjectAnalyzerResult::iter rb, re;
    rb = group.results.begin(); re = group.results.end();

    lua_newtable(lua);
    while(rb != re) {
        lua_pushstring(lua, rb->parameter.test_parameter_code.c_str());
            lua_newtable(lua);
                lua_pushstring(lua, rb->val.c_str());
                lua_setfield(lua, -2, "value");

                lua_pushstring(lua, rb->units.c_str());
                lua_setfield(lua, -2, "units");

                lua_pushstring(lua, rb->flags.c_str());
                lua_setfield(lua, -2, "flags");

                lua_pushstring(lua, rb->status.c_str());
                lua_setfield(lua, -2, "status");

                lua_pushstring(lua, rb->parameter.expression.c_str());
                lua_setfield(lua, -2, "expression");
        lua_settable(lua, -3);
        ++rb;
    }
    lua_setglobal(lua, "results");
}

void InterpreterHolder::setExpressionValue(const ObjectAnalyzerResult &result)
{
    value = result.val;

    lua_pushstring(lua, result.val.c_str());
    lua_setglobal(lua, "value");

    lua_pushstring(lua, result.units.c_str());
    lua_setglobal(lua, "units");

    if(luaL_dostring(lua, result.parameter.expression.c_str())) {
        const char *errorMessage = lua_tostring(lua, -1);
        lua_pop(lua, 1);
        throw InterpreterHolderException("exception when execute calculated fields for [%s]", string(errorMessage));
    }
}

const string& InterpreterHolder::getValue() const
{
    return value;
}

const string& InterpreterHolder::getUnit() const
{
    return unit;
}

int InterpreterHolder::luaCallBackSetValue(lua_State *state)
{
    int n = lua_gettop(state);
    if(n != 1) return 0;

    lua_getglobal(state, "_THIS");
    InterpreterHolder *_this = static_cast<InterpreterHolder*>(lua_touserdata(state, -1));

    if(!_this) return 0;

    switch(lua_type(state, 1)) {
        case LUA_TNUMBER:
        case LUA_TSTRING:
            _this->value = lua_tostring(state, 1);
            break;
        default:
            throw InterpreterHolderException("exception when receive set variable type!", "");
            break;
    }

    return 0;
}

int InterpreterHolder::luaCallBackSetUnit(lua_State *state)
{
    int n = lua_gettop(state);
    if(n != 1) return 0;

    lua_getglobal(state, "_THIS");
    InterpreterHolder *_this = static_cast<InterpreterHolder*>(lua_touserdata(state, -1));

    if(!_this) return 0;

    switch(lua_type(state, 1)) {
        case LUA_TSTRING:
            _this->unit = lua_tostring(state, 1);
            break;
        default:
            throw InterpreterHolderException("exception when receive set variable type!", "");
            break;
    }

    return 0;
}
