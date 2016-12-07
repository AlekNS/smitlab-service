
#ifndef ANALYZER_STORAGE_H
#define ANALYZER_STORAGE_H


#define ANALYZERS_STORAGE_VERSION_STRING "0.0.3"


#include <vector>
#include <string>


#include <Poco/SharedPtr.h>


namespace Poco { 
    class Mutex;
    class FastMutex;
    namespace Data { class Session; } 
}


using std::string;
using std::vector;
using std::map;

using Poco::SharedPtr;
using Poco::Mutex;
using Poco::FastMutex;
using Poco::Data::Session;


struct ObjectAnalyzerResult;
struct ObjectAnalyzerResultGroup;
struct ObjectAnalyzerSample;
struct ObjectAnalyzerPlace;


//============================================================================//
struct RowAnalyzerConfig
{
    typedef vector<RowAnalyzerConfig>::iterator iter;

    string                      code;
    string                      value;
};

//============================================================================//
struct RowAnalyzerParameter
{
    typedef vector<RowAnalyzerParameter>::iterator iter;

    int                         id;
    string                      code;
    string                      research_code;
    string                      test_code;
    string                      test_parameter_code;
    int                         display_index;
    int                         type;
    string                      expression;
};

//============================================================================//
struct RowAnalyzerPlace
{
    typedef vector<RowAnalyzerPlace>::iterator iter;

    int                         id;
    string                      code;
    string                      title;
};

//============================================================================//
struct RowAnalyzerPlaceParameter
{
    typedef vector<RowAnalyzerPlaceParameter>::iterator iter;

    int                         id;
    int                         place_id;
    string                      place_code;
    string                      code;
    string                      value;
};

//============================================================================//
struct RowAnalyzerSample
{
    typedef vector<RowAnalyzerSample>::iterator iter;

    string                      uid;

    string                      patient_uid;
    string                      patient_sex;
    string                      patient_birthday;
    string                      patient_fullname;

    int                         place_id;
    string                      place_stamp;
    string                      status;
    string                      comment;
};

//============================================================================//
struct RowAnalyzerResultGroup
{
    int                         id;
    int                         place_id;
    string                      sample_uid;
    int                         measure_type;
    string                      measure_stamp;
    string                      status;
    string                      comment;
};

//============================================================================//
struct RowAnalyzerResult
{
    int                         id;
    int                         result_group_id;
    int                         parameter_id;
    string                      measure_stamp;
    string                      units;
    string                      flags;
    string                      status;
    string                      comment;
    string                      val;
};

//============================================================================//
struct ObjectAnalyzerResult : public RowAnalyzerResult
{
    typedef vector<ObjectAnalyzerResult>::iterator iter;
    RowAnalyzerParameter                parameter;
};


struct ObjectAnalyzerResultGroup : public RowAnalyzerResultGroup
{
    typedef vector<ObjectAnalyzerResultGroup>::iterator iter;
    vector<ObjectAnalyzerResult>    results;
};


struct ObjectAnalyzerSample : public RowAnalyzerSample
{
    typedef vector<ObjectAnalyzerSample>::iterator iter;
    vector<ObjectAnalyzerResultGroup>   groups;
};


struct ObjectAnalyzerPlace : RowAnalyzerPlace
{
    typedef vector<ObjectAnalyzerPlace>::iterator iter;

    vector<RowAnalyzerPlaceParameter>    parameters;
    vector<ObjectAnalyzerResultGroup>    groups;
    vector<ObjectAnalyzerSample>         samples;
};


class AnalyzerStorage
{
public:

    AnalyzerStorage(const string &path, bool useExtends);
    virtual ~AnalyzerStorage();

    bool isTransaction();
    void waitTransaction();
    void begin(bool isExclusive=false);
    void commit();
    void rollback();

    SharedPtr<Session>& session() { return _session; }



    // configs
    void setConfig(const string &code,
        const string &value, const bool &rewrite = false);
    void queryConfig(vector<RowAnalyzerConfig> &result,
        const string &code);


    // parameters
    int setParameter(const string &code, const string &researchCode,
        const string &testCode, const string &testParameterCode,
        const int &displayIndex, const int &type = 1,
        const string &expresiion = "", bool rewrite = false);

    void queryParameter(vector<RowAnalyzerParameter> &result,
        const string &code);

    // inner vars
    void setVar(const int &id, const string &value);
    void setVar(const map<int, string> &values);

    void getVar(const int &id, string &value);
    void getVar(const int &idFrom, const int &idTo, map<int, string> &values);

    // places
    int setPlace(const string &code, const string &title);

    void queryPlace(vector<RowAnalyzerPlace> &result, const string &code);


    int setPlaceParameter(const int &placeId,
        const string &code,
        const string &value);

    void queryPlaceParameter(vector<RowAnalyzerPlaceParameter> &result,
        const int &placeId,
        const string &code);

    void removePlaceParameter(const int &placeId,
        const string &code);


    // samples
    void addSample(const string &uid,
        const string &patientUid,
        const string &patientSex,
        const string &patientBirthday,
        const string &patientFullname,
        const int &placeId);

    void setSampleStatus(const string &uid,
        const string &status, const string &comment);

    bool moveSampleToPlace(const string &uid, const int &placeId);

    void querySample(vector<RowAnalyzerSample> &result,
        const int &placeId, const string &uid);

    void removeSample(const string &uid);


    // results groups
    void queryResultGroup(vector<ObjectAnalyzerResultGroup> &result,
        const string &placeCode,
        const int &resultGroupId,
        const string &sampleUid);

    int addResultGroup(const int &placeId,
        const int &measureType,
        const string &sampleUid);

    void setResultGroupStatus(const int &id,
        const string &status, const string &comment);

    bool moveResultGroupToPlace(const int &id, const int &placeId, const string &uid = "");

    void removeResultGroup(const int &id);


    // results
    /*
    void addResult(const int &resultGroupId,
        const int &parameterId,
        const string &units,
        const string &val,
        const string &flags,
        const string &status,
        const string &comment);
    */

    void setResult(const int &resultGroupId,
        const int &parameterId,
        const string &units,
        const string &val,
        const string &flags,
        const string &status,
        const string &comment);

    void removeResult(const int &resultGroupId,
        const int &parameterId);

    void queryPlaceDetailed(vector<ObjectAnalyzerPlace> &result,
        const string &placeCode, bool withResults = true, bool withParameters = true,
        bool withEmptyResults = true);

    const int getCacheId();

protected:
    void invalidateCacheId();

private:

    SharedPtr<FastMutex>                    _mutexScope, _mutexCache;
    SharedPtr<Mutex>                        _mutexTransaction;
    SharedPtr<Session>                      _session;

    string                                  _path;
    int	                                    _cacheId;

};


#endif
