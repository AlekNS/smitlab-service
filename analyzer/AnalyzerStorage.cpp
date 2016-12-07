
#include <common/stdafx.h>


#include <Poco/Thread.h>
#include <Poco/Logger.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/Statement.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/Mutex.h>


#include "../common/DbCreate.h"
#include "AnalyzerStorage.h"


using namespace std;


using namespace Poco;
using namespace Poco::Data;


using namespace common;


//============================================================================//
const char *__analyzerStorageLoggerName__ = "AnalyzerStorage";


//============================================================================//
AnalyzerStorage::AnalyzerStorage(const string &path, bool useExtends) :
    _session(new Session("SQLite", path)),
    _mutexTransaction(new Mutex()),
    _mutexScope(new FastMutex()),
    _mutexCache(new FastMutex()),
    _path(path),
    _cacheId(1)
{
    try {
        try {
            DBInit(*_session, useExtends);
        }
        catch(Exception &e) {
            e.rethrow();
        }
        try {
            begin(true);

            poco_information_f1(Logger::get(__analyzerStorageLoggerName__),
                "create if not exists database schema [%s]", _path);

            DBCreate(*_session);

            poco_information_f1(Logger::get(__analyzerStorageLoggerName__),
                "init database [%s]", _path);

            commit();
        }
        catch(Exception &e) {
            rollback();
            e.rethrow();
        }

        poco_information_f1(Logger::get(__analyzerStorageLoggerName__),
            "vacuum database [%s]", _path);

        DBVacuum(*_session);

    }
    catch(Exception &e) {
        poco_error_f2(Logger::get(__analyzerStorageLoggerName__),
            "can't create analyzer storage [%s] / [%s]",
            e.displayText(), _path);
        e.rethrow();
    }
}

AnalyzerStorage::~AnalyzerStorage()
{
}

//============================================================================//
bool AnalyzerStorage::isTransaction()
{
    return _session->isTransaction();
}

void AnalyzerStorage::waitTransaction()
{
    int count = 0;
    while(isTransaction()) {
        Thread::sleep(5);
        ++count;
        if(count > 1000) {
            poco_error_f1(Logger::get(__analyzerStorageLoggerName__),
                "fail to wait ending of the db transaction [%s]",
                 _path);
            throw new IOException();
        }
    }
}

//============================================================================//
void AnalyzerStorage::begin(bool isExclusive)
{
    waitTransaction();

    {
        ScopedLock<FastMutex> lock(*_mutexScope);
        if(isExclusive)
            *_session << "begin exclusive transaction", now;
        else
            *_session << "begin transaction", now;
    }
}

//============================================================================//
void AnalyzerStorage::commit()
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    *_session << "commit", now;
}

//============================================================================//
void AnalyzerStorage::rollback()
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    *_session << "rollback", now;
}

//============================================================================//
const int AnalyzerStorage::getCacheId()
{
    ScopedLock<FastMutex> lock(*_mutexCache);

    return _cacheId;
}

//============================================================================//
void AnalyzerStorage::invalidateCacheId()
{
    ScopedLock<FastMutex> lock(*_mutexCache);

    _cacheId += 1;
}

//============================================================================//
void AnalyzerStorage::setConfig(const string &code,
    const string &value, const bool &rewrite)
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    try {

        Statement save(*_session);

        save << "insert or ";

        if(rewrite) save << " replace ";
        else        save << " ignore  ";

        save << " into configs (code, value) values (?, ?)",
            use(code), use(value);

        save.execute();
    }
    catch(Exception &e) {
        poco_error_f2(Logger::get(__analyzerStorageLoggerName__),
            "fail to setup config [%s] / [%s]",
            e.displayText(), _path);
        e.rethrow();
    }

    // save.done();
}

//============================================================================//
void AnalyzerStorage::queryConfig(vector<RowAnalyzerConfig> &result,
    const string &code)
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    try {

        Statement sel(*_session);

        sel << "select code, value from configs ";
        if(code.size()) {
            sel << " where code like ? ", use(code);
        }
        sel << " order by code";
        sel.execute();

        RecordSet rs(sel);

        if(rs.moveFirst()) {
            do {

                int  cinx = -1;
                RowAnalyzerConfig row;

                row.code  = rs[++cinx].extract<string>();
                row.value = rs[++cinx].extract<string>();

                result.push_back(row);

            } while(rs.moveNext());
        }
    }
    catch(Exception &e) {
        poco_error_f2(Logger::get(__analyzerStorageLoggerName__),
            "fail to query configs [%s] / [%s]",
            e.displayText(), _path);
        e.rethrow();
    }
}

//============================================================================//
int AnalyzerStorage::setParameter(const string &code, const string &researchCode,
    const string &testCode, const string &testParameterCode, const int &displayIndex,
    const int &type, const string &expression, bool rewrite)
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    try {


        int id = 0, count = 0;

        Statement ins(*_session);

        ins << "insert or ";

        if(rewrite)
            ins << "replace"; 
        else
            ins << "ignore"; 

        ins << " into parameters (code, research_code, test_code, test_parameter_code, "
                "display_index, type, expression) values (?, ?, ?, ?, ?, ?, ?) ",
                use(code), use(researchCode), use(testCode),
                use(testParameterCode), use(displayIndex), use(type), use(expression),
                now;

        // ins.done();

        Statement sel(*_session);
        sel << "select id from parameters where rowid = last_insert_rowid()", into(id), now;

        invalidateCacheId();

        return id;
    }
    catch(Exception &e) {
        poco_error_f3(Logger::get(__analyzerStorageLoggerName__),
            "fail to setup parameter [%s]:[%s] / [%s]",
            code, e.displayText(), _path);
        e.rethrow();
    }
    return 0;
}

//============================================================================//
void AnalyzerStorage::queryParameter(vector<RowAnalyzerParameter> &result,
    const string &code)
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    try {

        Statement sel(*_session);

        sel << "select id, code, research_code, test_code, test_parameter_code, display_index, "
                " type, expression "
                " from parameters ";
        if(code.size()) {
            sel << " where code like ? ", use(code);
        }
        sel << " order by display_index";
        sel.execute();

        // sel.done();

        RecordSet rs(sel);

        if(rs.moveFirst()) {

            do {
                int cinx = -1;
                RowAnalyzerParameter row;

                row.id              = rs[++cinx].extract<int>();
                row.code            = rs[++cinx].extract<string>();
                row.research_code   = rs[++cinx].extract<string>();
                row.test_code       = rs[++cinx].extract<string>();
                row.test_parameter_code = rs[++cinx].extract<string>();
                row.display_index   = rs[++cinx].extract<int>();
                row.type            = rs[++cinx].extract<int>();
                row.expression      = rs[++cinx].extract<string>();

                result.push_back(row);

            } while(rs.moveNext());

        }
    }
    catch(Exception &e) {
        poco_error_f2(Logger::get(__analyzerStorageLoggerName__),
            "fail to query parameters [%s] / [%s]",
            e.displayText(), _path);
        e.rethrow();
    }
}


//============================================================================//
void AnalyzerStorage::setVar(const int &id, const string &value)
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    try {

        *_session << "insert or replace into inner_vars(id, value) values(?, ?)",
            use(id), use(value), now;
    }
    catch(Exception &e) {
        poco_error_f2(Logger::get(__analyzerStorageLoggerName__),
            "fail to set inner variable [%s] / [%d]",
            e.displayText(), id);
        e.rethrow();
    }
}

//============================================================================//
void AnalyzerStorage::setVar(const map<int, string> &values)
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    if(!values.size()) return;

    try {
        int count = 0;

        *_session << "insert or replace into inner_vars(id, value) values ";
        for(map<int, string>::const_iterator iter = values.begin();
                iter != values.end();
                ++iter, ++count) {

                if(count) { *_session << ","; }

                *_session << "(?, ?)", use(iter->first), use(iter->second);
        }

        *_session, now;

    }
    catch(Exception &e) {
        poco_error_f1(Logger::get(__analyzerStorageLoggerName__),
            "fail to set inner variables [%s]",
            e.displayText());
        e.rethrow();
    }
}
    
//============================================================================//
void AnalyzerStorage::getVar(const int &id, string &value)
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    try {

        *_session << "select value from inner_vars where id = ?", into(value), use(id), now;
    }
    catch(Exception &e) {
        poco_error_f2(Logger::get(__analyzerStorageLoggerName__),
            "fail to get inner variable [%s] / [%d]",
            e.displayText(), id);
        e.rethrow();
    }
}
    
//============================================================================//
void AnalyzerStorage::getVar(const int &idFrom, const int &idTo, map<int, string> &values)
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    try {
        Statement sel(*_session);

        sel << "select id, value from inner_vars where id >= ? and id <= ?",
            use(idFrom), use(idTo);
        sel.execute();

        // sel.done();

        RecordSet rs(sel);

        if(rs.moveFirst()) {

            do {
                int cinx = -1;

                int     id = rs[++cinx].extract<int>();

                string val = rs[++cinx].extract<string>();
                values.insert(make_pair(id, val));

            } while(rs.moveNext());
        }
    }
    catch(Exception &e) {
        poco_error_f3(Logger::get(__analyzerStorageLoggerName__),
            "fail to get inner variables [%s] / [%d..%d]",
            e.displayText(), idFrom, idTo);
        e.rethrow();
    }
}


//============================================================================//
int AnalyzerStorage::setPlace(const string &code, const string &title)
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    try {

        int id = 0, count = 0;

        Statement ins(*_session);

        ins << "insert or ignore into places (code, title) "
                " values (?, ?) ",
                use(code), use(title), now;

        // ins.done();

        Statement sel(*_session);
        sel << "select id from places where rowid = last_insert_rowid()", into(id), now;

        invalidateCacheId();

        return id;
    }
    catch(Exception &e) {
        poco_error_f2(Logger::get(__analyzerStorageLoggerName__),
            "fail to setup place [%s] / [%s]",
            e.displayText(), _path);
        e.rethrow();
    }
    return 0;
}

//============================================================================//
void AnalyzerStorage::queryPlace(vector<RowAnalyzerPlace> &result,
    const string &code)
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    try {

        Statement sel(*_session);

        sel << "select id, code, title from places ";
        if(code.size() > 0) {
            sel << " where code like ? ", use(code);
        }
        sel << " order by code";
        sel.execute();

        // sel.done();

        RecordSet rs(sel);

        if(rs.moveFirst()) {

            do {
                int cinx = -1;
                RowAnalyzerPlace row;

                row.id              = rs[++cinx].extract<int>();
                row.code            = rs[++cinx].extract<string>();
                row.title           = rs[++cinx].extract<string>();

                result.push_back(row);
            } while(rs.moveNext());
        }
    }
    catch(Exception &e) {
        poco_error_f2(Logger::get(__analyzerStorageLoggerName__),
            "fail query places [%s] / [%s]",
            e.displayText(), _path);
        e.rethrow();
    }

}




//============================================================================//
int AnalyzerStorage::setPlaceParameter(const int &placeId,
    const string &code,
    const string &value)
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    try {

        int id = 0, count = 0;

        Statement upd(*_session);

        upd << "update places_parameters set value = ? where place_id = ? and code = ?",
            use(value), use(placeId), use(code), now;

        Statement sel(*_session);
        sel << "select changes()", into(count), now;

        if(!count) {

            Statement ins(*_session);

            ins << "insert into places_parameters (place_id, code, value) ";
            ins << "values (?, ?, ?)", use(placeId), use(code), use(value), now;

            // ins.done();

            sel = Statement(*_session);
            sel << "select id from places_parameters where rowid = last_insert_rowid()", into(id), now;

            // sel.done();
        }
        else {
            sel = Statement(*_session);
            sel << "select id from places_parameters where place_id = ? and code = ? limit 1",
                use(placeId), use(code), into(id), now;
        }

        invalidateCacheId();

        return id;
    }
    catch(Exception &e) {
        poco_error_f2(Logger::get(__analyzerStorageLoggerName__),
            "fail to setup place parameter [%s] / [%s]",
            e.displayText(), _path);
        e.rethrow();
    }

    return 0;
}

//============================================================================//
void AnalyzerStorage::queryPlaceParameter(vector<RowAnalyzerPlaceParameter> &result,
    const int &placeId,
    const string &code)
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    try {

        Statement sel(*_session);

        sel << "select pp.id, pp.place_id, pp.code, pp.value, p.code "
                " from places_parameters as pp join places as p on "
                " p.id = pp.place_id ";
        if(code.size()) {
            sel << " where code like ? ", use(code);
        }
        if(placeId) {
            if(!code.size()) sel << "where ";
            else sel << " and ";
            sel << " place_id = ?", use(placeId);
        }

        sel << " order by p.code, pp.code ";

        sel.execute();

        // sel.done();

        RecordSet rs(sel);

        if(rs.moveFirst()) {
            do {

                int cinx = -1;
                RowAnalyzerPlaceParameter row;

                row.id              = rs[++cinx].extract<int>();
                row.place_id        = rs[++cinx].extract<int>();
                row.code            = rs[++cinx].extract<string>();
                row.value           = rs[++cinx].extract<string>();
                row.place_code      = rs[++cinx].extract<string>();

                result.push_back(row);

            } while(rs.moveNext());
        }
    }
    catch(Exception &e) {
        poco_error_f2(Logger::get(__analyzerStorageLoggerName__),
            "fail to query places parameters [%s] / [%s]",
            e.displayText(), _path);
        e.rethrow();
    }
}

//============================================================================//
void AnalyzerStorage::removePlaceParameter(const int &placeId,
    const string &code)
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    try {

        Statement del(*_session);

        del << "delete from places_parameters where place_id = ? ", use(placeId);
        if(code.size()) {
            del << " and code like ? ", use(code);
        }
        del.execute();

        invalidateCacheId();
    }
    catch(Exception &e) {
        poco_error_f2(Logger::get(__analyzerStorageLoggerName__),
            "fail to remove place parameter [%s] / [%s]",
            e.displayText(), _path);
        e.rethrow();
    }

    // del.done();
}




//============================================================================//
void AnalyzerStorage::addSample(const string &uid,
    const string &patientUid, 
        const string &patientSex,
        const string &patientBirthday,
        const string &patientFullname,
    const int &placeId)
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    try {

        Statement ins(*_session);

        ins << "insert into samples (uid, patient_uid, patient_sex, patient_birthday, patient_fullname, place_id) "
                "values (?, ?, ?, ?, ?, ?)", use(uid), 
                use(patientUid), 
                use(patientSex),
                use(patientBirthday),
                use(patientFullname),
                use(placeId), 
                now;

        invalidateCacheId();
    }
    catch(Exception &e) {
        poco_error_f2(Logger::get(__analyzerStorageLoggerName__),
            "fail to add sample [%s] / [%s]",
            e.displayText(), _path);
        e.rethrow();
    }

    // ins.done();
}

//============================================================================//
void AnalyzerStorage::setSampleStatus(const string &uid,
    const string &status,
    const string &comment)
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    try {
        Statement save(*_session);

        save << "update samples set status = ?, comment = ? "
                "where uid = ?", use(status), use(comment), use(uid), now;

        invalidateCacheId();
    }
    catch(Exception &e) {
        poco_error_f2(Logger::get(__analyzerStorageLoggerName__),
            "fail to setup sample status [%s] / [%s]",
            e.displayText(), _path);
        e.rethrow();
    }
    // save.done();
}

//============================================================================//
bool AnalyzerStorage::moveSampleToPlace(const string &srcSampleUid, const int &dstPlaceId)
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    try {
        Statement save(*_session);
        Statement sel(*_session);

        string    dstSampleUid;
        int       srcPlaceId;
        const int temporaryPlaceId = 0xFFFF;

        // get current sample place
        *_session << "select place_id from samples where uid = ?", use(srcSampleUid), 
            into(srcPlaceId), now;

        // check exiting of sample in specified place
        *_session << "select uid from samples where place_id = ?", use(dstPlaceId), 
            into(dstSampleUid), now;

        if(dstPlaceId == srcPlaceId)
            return true;

        // update place parameters
        *_session << "update places_parameters set place_id = ? where place_id = ?",
            use(temporaryPlaceId), use(srcPlaceId);

        // if place filled sample
        if(dstSampleUid.size() > 0) {
            // move results groups to temporary place
            *_session << "update results_groups set place_id = ? "
                "where place_id = ? and sample_uid = ?",
                use(temporaryPlaceId),
                use(dstPlaceId), use(dstSampleUid), now;

            // move samples to temporary place
            *_session << "update samples set place_id = ? "
               "where place_id = ? and uid = ?",
                use(temporaryPlaceId),
                use(dstPlaceId), use(dstSampleUid), now;

            // update place parameters
            *_session << "update places_parameters set place_id = ? where place_id = ?",
                use(srcPlaceId), use(dstPlaceId);
        }

        // update all sample group results
        *_session << "update results_groups set place_id = ? "
                "where place_id = ? and sample_uid = ?",
                use(dstPlaceId),
                use(srcPlaceId), use(srcSampleUid), now;

        // update sample
        *_session << "update samples set place_id = ? where uid = ?", 
                use(dstPlaceId), use(srcSampleUid), now;

        // update place parameters
        *_session << "update places_parameters set place_id = ? where place_id = ?",
            use(dstPlaceId), use(temporaryPlaceId);

        // if place filled sample
        if(dstSampleUid.size() > 0) {
            // move results group to source requested position
            *_session << "update results_groups set place_id = ? "
                "where place_id = ? and sample_uid = ?",
                use(srcPlaceId),
                use(temporaryPlaceId), use(dstSampleUid), now;

            // move samples to source requested position
            *_session << "update samples set place_id = ? "
               "where place_id = ? and uid = ?",
                use(srcPlaceId),
                use(temporaryPlaceId), use(dstSampleUid), now;
        }

        invalidateCacheId();
    }
    catch(Exception &e) {
        poco_error_f2(Logger::get(__analyzerStorageLoggerName__),
            "fail to move sample to another place [%s] / [%s]",
            e.displayText(), _path);
        e.rethrow();
    }

    return true;
}

//============================================================================//
void AnalyzerStorage::querySample(vector<RowAnalyzerSample> &result,
    const int &placeId, const string &uid)
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    try {

        Statement sel(*_session);

        sel << "select uid, patient_uid, place_id, datetime(place_stamp), status, comment "
                " from samples ";

        if(placeId > 0) {
            sel << " where place_id = ?", use(placeId);
        }

        if(uid.size() > 0) {
            if(placeId > 0)
                sel << " and ";
            else
                sel << " where ";
            sel << " uid like ? ", use(uid);
        }

        sel.execute();

        // sel.done();

        RecordSet rs(sel);

        if(rs.moveFirst()) {
            do {

                int cinx = -1;
                RowAnalyzerSample row;

                row.uid             = rs[++cinx].extract<string>();
                row.patient_uid     = rs[++cinx].extract<string>();
                row.place_id        = rs[++cinx].extract<int>();
                row.place_stamp     = rs[++cinx].extract<string>();
                row.status          = rs[++cinx].extract<string>();
                row.comment         = rs[++cinx].extract<string>();

                result.push_back(row);

            } while(rs.moveNext());
        }
    }
    catch(Exception &e) {
        poco_error_f2(Logger::get(__analyzerStorageLoggerName__),
            "fail to query samples [%s] / [%s]",
            e.displayText(), _path);
        e.rethrow();
    }

}

//============================================================================//
void AnalyzerStorage::removeSample(const string &uid)
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    try {

        Statement del(*_session);

        del << "delete from samples "
                "where uid = ?", use(uid), now;
        // del.done();
        invalidateCacheId();
    }
    catch(Exception &e) {
        poco_error_f2(Logger::get(__analyzerStorageLoggerName__),
            "fail to remove sample [%s] / [%s]",
            e.displayText(), _path);
        e.rethrow();
    }

}




//============================================================================//
int AnalyzerStorage::addResultGroup(const int &placeId,
    const int &measureTyped,
    const string &sampleUid)
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    try {

        int id = 0;

        if(sampleUid.size() > 0) {
            Statement _statementResultGroupForSample(*_session);
            _statementResultGroupForSample <<
                "insert into results_groups (place_id, sample_uid, measure_type)    "
                "values (?, ?, ?)                                                   "
            ;

            _statementResultGroupForSample,
                use(placeId), use(sampleUid), use(measureTyped);

            _statementResultGroupForSample.execute();

        }
        else {
            Statement _statementResultGroupAdd(*_session);
            _statementResultGroupAdd <<
                "insert into results_groups (place_id, measure_type)    "
                "values (?, ?)                                          "
            ;

            _statementResultGroupAdd,
                use(placeId), use(measureTyped);
            _statementResultGroupAdd.execute();
        }

        Statement _statementResultGroupGetLastId(*_session);
        _statementResultGroupGetLastId <<
            "select id from results_groups where rowid = last_insert_rowid()"
        ;

        _statementResultGroupGetLastId, into(id);
        _statementResultGroupGetLastId.execute();

        invalidateCacheId();

        // _statementResultGroupGetLastId.done();
        return id;
    }
    catch(Exception &e) {
        poco_error_f2(Logger::get(__analyzerStorageLoggerName__),
            "fail to add result group [%s] / [%s]",
            e.displayText(), _path);
        e.rethrow();
    }

    return 0;
}

//============================================================================//
void AnalyzerStorage::setResultGroupStatus(const int &id,
    const string &status,
    const string &comment)
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    try {

        Statement _statementResultGroupSetStatus(*_session);

        _statementResultGroupSetStatus <<
            "update results_groups set status = ?, comment = ?      "
            "where id = ?                                           "
        ;

        _statementResultGroupSetStatus,
            use(status), use(comment), use(id);
        _statementResultGroupSetStatus.execute();

        invalidateCacheId();
    }
    catch(Exception &e) {
        poco_error_f2(Logger::get(__analyzerStorageLoggerName__),
            "fail to setup result group status [%s] / [%s]",
            e.displayText(), _path);
        e.rethrow();
    }

    // save.done();
}

//============================================================================//
bool AnalyzerStorage::moveResultGroupToPlace(const int &srcGroupId, const int &dstPlaceId, 
    const string &dstSampleUid)
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    int       srcPlaceId;
    string    srcSampleUid;

    try {
        *_session << "select place_id, sample_uid from results_groups where id = ?", 
            use(srcGroupId), into(srcPlaceId), into(srcSampleUid), now;

        if(srcPlaceId == dstPlaceId && !dstSampleUid.compare(srcSampleUid))
            return true;

        // if results group move to sample results groups section
        *_session << "update results_groups set sample_uid = ?, place_id = ? "
            "where id = ?", 
            use(dstSampleUid), use(dstPlaceId), use(srcGroupId), now;

        invalidateCacheId();
    }
    catch(Exception &e) {
        poco_error_f2(Logger::get(__analyzerStorageLoggerName__),
            "fail to move result group to another place [%s] / [%s]",
            e.displayText(), _path);
        e.rethrow();
    }
    
    return true;
}


//============================================================================//
void AnalyzerStorage::removeResultGroup(const int &id)
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    try {

        Statement del(*_session);

        del << "delete from results_groups "
                "where id = ?", use(id), now;

        invalidateCacheId();
    }
    catch(Exception &e) {
        poco_error_f2(Logger::get(__analyzerStorageLoggerName__),
            "fail to remove result group [%s] / [%s]",
            e.displayText(), _path);
        e.rethrow();
    }

    // del.done();
}



//============================================================================//
/*

void AnalyzerStorage::addResult(const int &resultGroupId,
        const int &parameterId,
        const string &units,
        const string &val,
        const string &flags,
        const string &status,
        const string &comment)
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    try {

        Statement _statementResultAdd(*_session);

        _statementResultAdd <<
            "insert into results (  "
            "   result_group_id,    "
            "   parameter_id,       "
            "   units,              "
            "   flags,              "
            "   status,             "
            "   comment,            "
            "   val                 "
            ")                      "
            "values (?, ?, ?, ?, ?, ?, ?)"
        ;

        _statementResultAdd, use(resultGroupId), use(parameterId), use(units),
                use(flags), use(status), use(comment), use(val),
                now;

	invalidateCacheId();
    }
    catch(Exception &e) {
        poco_error_f2(Logger::get(__analyzerStoragLoggerName__),
            "fail to add result [%s] / [%s]",
            e.displayText(), _path);
        e.rethrow();
    }

    // ins.done();
}

*/



// @TODO: Should be refactor
//============================================================================//
void AnalyzerStorage::setResult(const int &resultGroupId,
        const int &parameterId,
        const string &units,
        const string &val,
        const string &flags,
        const string &status,
        const string &comment)
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    int resultId = 0;

    try {

        Statement _statementResultQuery(*_session);

        *_session << "select coalesce(id, 0) from results where result_group_id = ? and parameter_id = ?", 
            use(resultGroupId), use(parameterId), into(resultId), now;

        Statement _statementResultSet(*_session);

        _statementResultSet <<
            "insert                 "
        ;

        if(resultId)
            _statementResultSet <<
                " or replace            "
            ;

        _statementResultSet << "into results (  ";

        if(resultId) 
            _statementResultSet << 
                "   id,                 "
            ;

        _statementResultSet << 
            "   result_group_id,    "
            "   parameter_id,       "
            "   units,              "
            "   flags,              "
            "   status,             "
            "   comment,            "
            "   val                 "
            ")                      "
            "values (               ";

        if(resultId) {
            _statementResultSet <<
            "?, "
            ;
        }

        _statementResultSet << 
            "?, ?, ?, ?, ?, ?, ?)"
        ;

        if(resultId) {
            _statementResultSet, use(resultId);
        }

        _statementResultSet, use(resultGroupId), use(parameterId), use(units),
                use(flags), use(status), use(comment), use(val),
                now;

        invalidateCacheId();
    }
    catch(Exception &e) {
        poco_error_f2(Logger::get(__analyzerStorageLoggerName__),
            "fail to set result [%s] / [%s]",
            e.displayText(), _path);
        e.rethrow();
    }

    // ins.done();
}


//============================================================================//
void AnalyzerStorage::removeResult(const int &resultGroupId,
        const int &parameterId)
{
    ScopedLock<FastMutex> lock(*_mutexScope);

    try {

        Statement _statementResultRemove(*_session);

        _statementResultRemove <<
            "delete from results    "
            "where                  "
            "   result_group_id = ? and "
            "   parameter_id = ?    "
        ;

        _statementResultRemove, use(resultGroupId), use(parameterId),
            now;

        invalidateCacheId();
    }
    catch(Exception &e) {
        poco_error_f2(Logger::get(__analyzerStorageLoggerName__),
            "fail to remove result [%s] / [%s]",
            e.displayText(), _path);
        e.rethrow();
    }

    // ins.done();
}

// @TODO: Should be refactor
//============================================================================//
void AnalyzerStorage::queryPlaceDetailed(vector<ObjectAnalyzerPlace> &result,
    const string &placeCode,
    bool withResults, bool withParameters,
    bool withEmptyResults)
{

    ScopedLock<FastMutex> lock(*_mutexScope);

    try {

        Statement sel(*_session);

        const int           placeInx = 0,
                            resultGroupInx = 2,
                            sampleInx = 7,
                            placeParamInx = 12 + 3,
                            paramInx  = 12 + 3,
                            resultInx = 16 + 3;


        // @TODO: Refactor this fucking horror!!!

        static const char* queryFieldsCommon =
            "select                                                             "
                // places
            "    p.code, p.title,                                               " // 0 ..
                // results groups
            "    coalesce(rg.id, 0) as rg_id, coalesce(rg.measure_type, -1),    "         // 2
            "    datetime(rg.measure_stamp) as rgmeasure_stamp,                 "
            "    rg.status, rg.comment,                                         "
                // samples
            "    s.uid, s.patient_uid, s.patient_sex, date(s.patient_birthday), s.patient_fullname, " // 7
            "    datetime(s.place_stamp), s.status, s.comment     "
        ;

        static const char* queryFieldsResults =                                                //
            "    ,prm.code, prm.research_code, prm.test_code, prm.test_parameter_code,      "  // 15 ..
            "     datetime(r.measure_stamp), r.flags, r.units, coalesce(r.status, 'empty'), r.comment, r.val,  "  // 19 ..
            "     prm.display_index as prmdisplay_index, prm.type as prmtype,               "
            "     prm.expression as prmexpr                                                 "
        ;

        static const char* leftJoinResults =
            "left  join parameters as prm on                                            "
            "    rg.id notnull                                                          "
            "left  join results as r on                                                 "
            "    r.result_group_id = rg.id and                                          "
            "    r.parameter_id = prm.id                                                "
        ;

        //
        // Queries...
        //

        // Unassigned results group
        sel << queryFieldsCommon;

        if(withResults)     sel << queryFieldsResults;

        // from with joins
        sel <<
            "from places as p                                                           "
            "left  join results_groups as rg on                                         "
            "    rg.place_id = p.id and rg.sample_uid = ''                              "
        ;

        if(withResults)     sel << leftJoinResults;

        // dummy columns for union selecting
        sel << "left  join samples as s on s.uid = '' ";

        if(placeCode.size() > 0) {
            sel << " where p.code like ? ",
                use(placeCode);
        }

        //
        // Assigned results groups with samples
        //
        sel << "union " << queryFieldsCommon;
        if(withResults) sel << queryFieldsResults;

        // from with joins
        sel <<
            "from places as p                                                           "
            "left  join samples as s on                                                 "
            "    s.place_id = p.id                                                      "
            "left  join results_groups as rg on                                         "
            "    rg.place_id = p.id and s.uid = rg.sample_uid                           "
        ;

        if(withResults)     sel << leftJoinResults;


        if(placeCode.size() > 0) {
            sel << " where p.code like ?                                                    ",
                use(placeCode);
        }

        // special sorting for mapping results into exists structure
        sel <<
            "order by                                                                   "
            "    p.code                                                                 "
            "    ,s.uid                                                                 "
        ;

        if(withResults) {
            sel <<
                "    ,rgmeasure_stamp desc,                                                "
                "    rg_id,                                                                "
                "    prmdisplay_index                                                      "
            ;
        }

        sel << " limit 10240";

        sel.execute();
        // poco_assert(sel.done()); @TODO: Append statement execution checking!

        RecordSet rs(sel);
        map<string, ObjectAnalyzerPlace* >              placesMap;
        map<string, ObjectAnalyzerPlace* >::iterator    placeMap;

        if(rs.moveFirst()) {

            // to check what is change in rows, not safe in common case
            int                 rgId         = -1;
            string              place        = "###???",
                                sampleUid    = "###???",
                                measureStamp = "###???",
                                placeParamCode = "###???";

            // pointers to create objects
            ObjectAnalyzerPlace         *currentPlace       = NULL;
            ObjectAnalyzerSample        *currentSample      = NULL;
            ObjectAnalyzerResultGroup   *currentResultGroup = NULL;

            // common column index
            int                         cinx;

            do {
                // iterate over place
                if(rs[placeInx].convert<string>() != place) {
                    place = rs[placeInx].convert<string>();

                    cinx = placeInx - 1;

                    ObjectAnalyzerPlace plc;

                    // read place
                    plc.code  = rs[++cinx].convert<string>();
                    plc.title = rs[++cinx].convert<string>();

                    result.push_back(plc);
                    currentPlace = &result[result.size() - 1];
                }

                // iterate over sample
                if(rs[sampleInx].convert<string>().size() > 0) {
                    if(rs[sampleInx].convert<string>().compare(sampleUid)) {
                        sampleUid = rs[sampleInx].convert<string>();


                        cinx = sampleInx - 1;

                        ObjectAnalyzerSample        sample;

                        // read sample
                        sample.uid         = rs[++cinx].convert<string>();
                        sample.patient_uid = rs[++cinx].convert<string>();
                        sample.patient_sex = rs[++cinx].convert<string>();
                        sample.patient_birthday = rs[++cinx].convert<string>();
                        sample.patient_fullname = rs[++cinx].convert<string>();
                        sample.place_stamp = rs[++cinx].convert<string>();
                        sample.status      = rs[++cinx].convert<string>();
                        sample.comment     = rs[++cinx].convert<string>();

                        currentPlace->samples.push_back(sample);

                        currentSample = &currentPlace->samples[currentPlace->samples.size() - 1];
                    }
                }
                else {
                    currentSample = NULL;
                }

                // iterate over results groups
                if(rs[resultGroupInx].convert<int>() != rgId) {

                    if(rs[resultGroupInx].convert<int>()) {

                        rgId = rs[resultGroupInx].convert<int>();

                        ObjectAnalyzerResultGroup group;


                        cinx = resultGroupInx - 1;

                        // read group
                        group.id            = rs[++cinx].convert<int>();
                        group.measure_type  = rs[++cinx].convert<int>();
                        group.measure_stamp = rs[++cinx].convert<string>();
                        group.status        = rs[++cinx].convert<string>();
                        group.comment       = rs[++cinx].convert<string>();

                        // if result group assigned to current sample
                        if(currentSample) {
                            currentSample->groups.push_back(group);
                            currentResultGroup = &currentSample->groups[currentSample->groups.size() - 1];
                        }
                        else {
                            currentPlace->groups.push_back(group);
                            currentResultGroup = &currentPlace->groups[currentPlace->groups.size() - 1];
                        }

                    }
                    else {
                        currentResultGroup = NULL;
                    }
                }

                // append results if need
                if(withResults && currentResultGroup) {

                    ObjectAnalyzerResult res;


                    cinx = paramInx - 1;

                    // read parameter
                    res.parameter.code                = rs[++cinx].convert<string>();
                    res.parameter.research_code       = rs[++cinx].convert<string>();
                    res.parameter.test_code           = rs[++cinx].convert<string>();
                    res.parameter.test_parameter_code = rs[++cinx].convert<string>();


                    cinx = resultInx - 1;

                    // read result
                    res.measure_stamp = rs[++cinx].convert<string>();
                    res.flags         = rs[++cinx].convert<string>();
                    res.units         = rs[++cinx].convert<string>();
                    res.status        = rs[++cinx].convert<string>();
                    res.comment       = rs[++cinx].convert<string>();
                    res.val           = rs[++cinx].convert<string>();
                    ++cinx; // skip display index

                    // read parameter continue...
                    res.parameter.type          = rs[++cinx].convert<int>();
                    res.parameter.expression    = rs[++cinx].convert<string>();

                    currentResultGroup->results.push_back(res);
                }

            } while(rs.moveNext());

            // get places 
            if(withParameters) {
                vector<RowAnalyzerPlaceParameter> placeParams;

                try {

                    Statement sel(*_session);

                    sel << "select pp.id, pp.place_id, pp.code, pp.value, p.code "
                            " from places_parameters as pp join places as p on "
                            " p.id = pp.place_id order by p.code, pp.code ";

                    sel.execute();

                    // sel.done();

                    RecordSet rs(sel);

                    if(rs.moveFirst()) {
                        do {

                            int cinx = -1;
                            RowAnalyzerPlaceParameter row;

                            row.id              = rs[++cinx].extract<int>();
                            row.place_id        = rs[++cinx].extract<int>();
                            row.code            = rs[++cinx].extract<string>();
                            row.value           = rs[++cinx].extract<string>();
                            row.place_code      = rs[++cinx].extract<string>();

                            placeParams.push_back(row);

                        } while(rs.moveNext());
                    }
                }
                catch(Exception &e) {
                    poco_error_f2(Logger::get(__analyzerStorageLoggerName__),
                        "fail to query places parameters [%s] / [%s]",
                        e.displayText(), _path);
                    e.rethrow();
                }

                // prepare places
                for(ObjectAnalyzerPlace::iter place = result.begin(); place != result.end(); ++place) {
                    placesMap.insert(make_pair(place->code, &(*place)));
                }

                for(RowAnalyzerPlaceParameter::iter placeParameter = placeParams.begin();
                        placeParameter != placeParams.end();
                        ++placeParameter)
                {
                    placeMap = placesMap.find(placeParameter->place_code);
    //                cout << placeParameter->place_code << endl;
                    if(placeMap != placesMap.end()) {
                        placeMap->second->parameters.push_back(*placeParameter);
    //                    cout << placeMap->second << " <- ptr " << endl;
    //                    cout << placeMap->second->parameters.size() << endl;
                    }
                }
            } // withParameters

        } // rs!?
    }
    catch(Exception &e) {
        poco_error_f2(Logger::get(__analyzerStorageLoggerName__),
            "fail to query place results [%s] / [%s]",
            string(e.displayText()), _path);
        e.rethrow();
    }

}


// @TODO: Should be refactor
//============================================================================//
void AnalyzerStorage::queryResultGroup(vector<ObjectAnalyzerResultGroup> &result,
        const string &placeCode,
        const int &resultGroupId,
        const string &sampleUid)
{

    ScopedLock<FastMutex> lock(*_mutexScope);

    try {

        Statement sel(*_session);

        int placeId = 0;

        if(placeCode.size()) {
            *_session << "select id from places where code = ?", use(placeCode), into(placeId), now;
        }

        const int           resultGroupInx = 0,
                            paramInx  = 5,
                            resultInx = 9;


        // @TODO: Refactor this fucking horror!!!

        static const char* queryFieldsCommon =
            "select                                                             "
                // results groups
            "    coalesce(rg.id, 0) as rg_id, coalesce(rg.measure_type, -1),    "         // 2
            "    datetime(rg.measure_stamp) as rgmeasure_stamp,                 "
            "    rg.status, rg.comment,                                         "
        ;

        static const char* queryFieldsResults =                                                //
            "     prm.code, prm.research_code, prm.test_code, prm.test_parameter_code,      "  // 5
            "     datetime(r.measure_stamp), r.flags, r.units, coalesce(r.status, 'empty'), r.comment, r.val,  "  // 9 ..
            "     prm.display_index as prmdisplay_index, prm.type as prmtype,               "
            "     prm.expression as prmexpr                                                 "
        ;

        static const char* leftJoinResults =
            "left  join parameters as prm on                                            "
            "    rg.id notnull                                                          "
            "inner join results as r on                                                 "
            "    r.result_group_id = rg.id and                                          "
            "    r.parameter_id = prm.id                                                "
        ;

        //
        // Queries...
        //

        // Unassigned results group
        sel << queryFieldsCommon;
        sel << queryFieldsResults;

        // from with joins
        sel <<
            "from results_groups as rg                                                  "
        ;

        sel << leftJoinResults;

        // rg.place_id = p.id and rg.sample_uid
        if(resultGroupId) {
            sel << " where rg.id = ?                                                      ",
                use(resultGroupId);
        }
        else if(sampleUid.size()) {
            sel << " where rg.sample_uid like ?                                           ",
                use(sampleUid);
        }
        else if(placeCode.size() > 0) {
            sel << " where rg.place_id = ?                                                ",
                use(placeId);
        }
        else {
            return;
        }

        // special sorting for mapping results into exists structure
        sel <<
            "    order by rgmeasure_stamp desc,                                        "
            "    rg_id,                                                                "
            "    prmdisplay_index                                                      "
        ;

        sel << " limit 10240";

        sel.execute();
        // poco_assert(sel.done()); @TODO: Append statement execution checking!

        RecordSet rs(sel);

        if(rs.moveFirst()) {

            // to check what is change in rows, not safe in common case
            int                 rgId         = -1;
            string              measureStamp = "###???",
                                placeParamCode = "###???";

            ObjectAnalyzerResultGroup   *currentResultGroup = NULL;

            // common column index
            int                         cinx;

            do {
                // iterate over results groups
                if(rs[resultGroupInx].convert<int>() != rgId) {

                    if(rs[resultGroupInx].convert<int>()) {

                        rgId = rs[resultGroupInx].convert<int>();

                        ObjectAnalyzerResultGroup group;


                        cinx = resultGroupInx - 1;

                        // read group
                        group.id            = rs[++cinx].convert<int>();
                        group.measure_type  = rs[++cinx].convert<int>();
                        group.measure_stamp = rs[++cinx].convert<string>();
                        group.status        = rs[++cinx].convert<string>();
                        group.comment       = rs[++cinx].convert<string>();

                        // if result group assigned to current sample
                        result.push_back(group);
                        currentResultGroup = &result[result.size() - 1];

                    }
                    else {
                        currentResultGroup = NULL;
                    }
                }

                // append results if need
                if(currentResultGroup) {

                    ObjectAnalyzerResult res;

                    cinx = paramInx - 1;

                    // read parameter
                    res.parameter.code                = rs[++cinx].convert<string>();
                    res.parameter.research_code       = rs[++cinx].convert<string>();
                    res.parameter.test_code           = rs[++cinx].convert<string>();
                    res.parameter.test_parameter_code = rs[++cinx].convert<string>();


                    cinx = resultInx - 1;

                    // read result
                    res.measure_stamp = rs[++cinx].convert<string>();
                    res.flags         = rs[++cinx].convert<string>();
                    res.units         = rs[++cinx].convert<string>();
                    res.status        = rs[++cinx].convert<string>();
                    res.comment       = rs[++cinx].convert<string>();
                    res.val           = rs[++cinx].convert<string>();
                    ++cinx; // skip display index

                    // read parameter continue...
                    res.parameter.type          = rs[++cinx].convert<int>();
                    res.parameter.expression    = rs[++cinx].convert<string>();

                    currentResultGroup->results.push_back(res);
                }

            } while(rs.moveNext());
        } // rs!?
    }
    catch(Exception &e) {
        poco_error_f2(Logger::get(__analyzerStorageLoggerName__),
            "fail to query place results [%s] / [%s]",
            string(e.displayText()), _path);
        e.rethrow();
    }

}
