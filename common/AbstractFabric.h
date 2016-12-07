
#ifndef ABSTRACT_FABRICK_H
#define ABSTRACT_FABRICK_H


#include <map>
#include <string>


namespace common {


using std::map;
using std::pair;
using std::string;


//================================================================================================//
struct ObjectInstantiatorEmptyArg { };


//================================================================================================//
template<typename Base,
    typename Argument>
class AbstractObjectInstantiator
{
public:
    AbstractObjectInstantiator() { }
    virtual ~AbstractObjectInstantiator() { }
    virtual Base *create(Argument &arg) const = 0;
};

template<typename Base>
class AbstractObjectInstantiator<Base, ObjectInstantiatorEmptyArg>
{
public:
    AbstractObjectInstantiator() { }
    virtual ~AbstractObjectInstantiator() { }
    virtual Base *create() const = 0;
};


//================================================================================================//
template<class C, class Base, class Argument>
class ObjectInstantiator : public AbstractObjectInstantiator<Base, Argument>
{
public:
    ObjectInstantiator() { }
    virtual ~ObjectInstantiator() { }

    virtual Base *create(Argument &arg) const { return new C(arg); }

};

template<typename C, typename Base>
class ObjectInstantiator<C, Base, ObjectInstantiatorEmptyArg> :
    public AbstractObjectInstantiator<Base, ObjectInstantiatorEmptyArg>
{
public:
    ObjectInstantiator() { }
    virtual ~ObjectInstantiator() { }

    virtual Base *create() const { return new C(); }

};


//================================================================================================//
template<class Base, class Argument = ObjectInstantiatorEmptyArg>
class AbstractFabric
{
    typedef AbstractObjectInstantiator<Base, Argument>          FabricType;
    typedef typename map<string, FabricType *>::iterator        FabricIter;

public:

    AbstractFabric() { }

    ~AbstractFabric()
    {
        for(FabricIter iter = _fabrics.begin(); iter != _fabrics.end(); ++iter)
            delete iter->second;
    }

    template <class C>
    void registerClass(const string &name)
    {
        // @TODO: Append mutex
        _fabrics.insert(pair<string, FabricType*>(name,
            new ObjectInstantiator<C, Base, Argument>()));
    }

    void unregisterClass(const string &name) {

        // @TODO: Append mutex
        FabricIter iter = _fabrics.find(name);
        if(iter != _fabrics.end()) {
            delete iter->second;
            _fabrics.erase(iter);
        }
    }

    const bool isClass(const string &name) const
    {
        // @TODO: Append mutex
        return _fabrics.find(name) != _fabrics.end();
    }


    Base *create(const string &name, Argument &arg)
    {
        FabricType      *fabric = getFabric(name);
        if(fabric) return fabric->create(arg);
        return NULL;
    }

    Base *create(const string &name)
    {
        FabricType      *fabric = getFabric(name);
        if(fabric) return fabric->create();
        return NULL;
    }

protected:

    FabricType* getFabric(const string &name)
    {
        FabricIter iter = _fabrics.find(name);
        if(iter != _fabrics.end()) {
            return iter->second;
        }
        return NULL;
    }

    map<string, FabricType*>                  _fabrics;

};


}


#endif
