
#include <common/stdafx.h>


#include <Poco/Data/Session.h>
#include <Poco/Data/SessionFactory.h>


#include "DbCreate.h"


using namespace Poco;
using namespace Poco::Data;


//============================================================================//
void common::DBInit(Session &session, bool useExtends)
{
    try {
        session << "PRAGMA synchronous = 0;", now;
        session << "PRAGMA page_size = 4096;", now;
        if(useExtends) {
            session << "PRAGMA journal_mode = WAL;", now;
        }
        session << "PRAGMA cache_size   = 16384;", now;
        session << "PRAGMA encoding     = \"UTF-8\";", now;
    }
    catch(...) {
        throw;
    }
}

//============================================================================//
void common::DBVacuum(Session &session)
{
    try {
        session << "VACUUM", now;
    }
    catch(...) {
        throw;
    }
}

//============================================================================//
void common::DBCreate(Session &session)
{

    try {
        /**
         * Description internal configurations of analyzer.
         * Example:
         *        code = type
         *        value = beckman_diff2
         */
        static const char *_sqlCreateConfiguration =
            "create table if not exists configs (                                   "
            "    code                    varchar(256)        primary key,           "
            "    value                   varchar(1024)       not null               "
            ")                                                                      "
        ;
        session << _sqlCreateConfiguration, now;

        /**
         * Description internal parameter conversion between analyzer and smitlab.
         * Example:
         *        code = HB
         *        test_code = COMMON_BLOOD
         *        test_parameter_code = HEMOGLOBIN
         */
        static const char *_sqlCreateParametersTable =
            "create table if not exists parameters (                                        "
            "    id                         integer         primary key autoincrement,      "
            "    code                       varchar(36)     not null,                       "
            "    research_code              varchar(36)     not null,                       "
            "    test_code                  varchar(36)     not null,                       "
            "    test_parameter_code        varchar(64)     not null,                       "
            "    display_index              integer         not null default 0,             "
            "    type                       integer         not null default 1,             "
            "    expression                 varchar(4096)   not null default ''             "
            ")                                                                              "
        ;
        session << _sqlCreateParametersTable, now;

        static const char *_sqlCreateParametersUniqueIndex =
            "create unique index if not exists uinx_parameters on parameters (code)         "
        ;
        session << _sqlCreateParametersUniqueIndex, now;

        /**
         * Description place representation.
         * Example:
         *        code = place_1
         *        title = 1
         */
        static const char *_sqlCreatePlacesTable =
            "create table if not exists places (                                                "
            "    id                         integer             primary key autoincrement,      "
            "    code                       varchar(36)         not null,                       "
            "    title                      varchar(64)         not null                        "
            ")                                                                                  "
        ;
        session << _sqlCreatePlacesTable, now;

        static const char *_sqlCreatePlacesUniqueIndex =
            "create unique index if not exists uinx_places on places (code)                     "
        ;
        session << _sqlCreatePlacesUniqueIndex, now;

        /**
         * Description place parameters.
         * Example:
         */
        static const char *_sqlCreatePlacesParametersTable =
            "create table if not exists places_parameters (                                     "
            "    id                         integer             primary key autoincrement,      "
            "    place_id                   integer             not null,                       "
            "    code                       varchar(64)         not null,                       "
            "    value                      varchar(256)        not null                        "
            ")                                                                                  "
        ;
        session << _sqlCreatePlacesParametersTable, now;

        /**
         * Description place parameters.
         * Example:
         */
        static const char *_sqlCreateInnerVariablesTable =
            "create table if not exists inner_vars (                                            "
            "    id                         integer             primary key,                    "
            "    value                      varchar(512)        not null                        "
            ")                                                                                  "
        ;
        session << _sqlCreateInnerVariablesTable, now;

        /**
         * Description sample info in the place.
         */
        static const char *_sqlCreateSamplesTable =
            "create table if not exists samples (                                   "
            "    uid                        varchar(36)         primary key asc,    "
            "    barcode                    varchar(36),                            "
            "    patient_uid                varchar(36)         not null,           "
            "    patient_sex                varchar(3)          not null,           "
            "    patient_birthday           varchar(8)          not null,           "
            "    patient_fullname           varchar(64)         not null,           "
            "    place_id                   integer             not null,           "
            "    place_stamp                datetime            not null default (datetime(current_timestamp, 'localtime')), "
            "                                                                       "
            "    status                     varchar(64)         not null default '',"
            "                                                                       "
            "    comment                    varchar(64)         not null default '' "
            ")                                                                      "
        ;
        session << _sqlCreateSamplesTable, now;

        static const char *_sqlCreateTriggerOnDeleteSampleWhenNotEmpty =
            "create trigger if not exists [delete_samples_when_not_empty] before delete "
            "on [samples]                                                               "
            "for each row when old.status <> ''                                         "
            "begin                                                                      "
            "                                                                           "
            "    update results_groups set sample_uid = ''                              "
            "    where sample_uid = old.uid;                                            "
            "                                                                           "
            "    delete from                                                            "
            "        results_groups                                                     "
            "    where                                                                  "
            "        sample_uid = old.uid AND                                           "
            "        place_id = old.place_id;                                           "
            "                                                                           "
            "end                                                                        "
        ;
        session << _sqlCreateTriggerOnDeleteSampleWhenNotEmpty, now;

        /**
         * Description measured results grouped by stamp.
         */
        static const char *_sqlCreateResultsGroups =
            "create table if not exists results_groups (                                    "
            "    id                         integer         primary key asc autoincrement,  "
            "    place_id                   integer         not null,                       "
            "    sample_uid                 varchar(36)     not null default '',            "
            "    measure_type               integer         not null default 0,             "
            "    measure_stamp              datetime        default (datetime(current_timestamp, 'localtime')), "
            "                                                                               "
            "    status                     varchar(64)     not null default '',            "
            "                                                                               "
            "    comment                    varchar(64)     not null default ''             "
            ")                                                                              "
        ;
        session << _sqlCreateResultsGroups, now;

        static const char *_sqlCreateTableOnDeleteResultsGroups =
            "create trigger if not exists [delete_results_groups] before delete         "
            "on [results_groups]                                                        "
            "for each row                                                               "
            "begin                                                                      "
            "                                                                           "
            "    delete from results where result_group_id = old.id;                    "
            "                                                                           "
            "end                                                                        "
        ;
        session << _sqlCreateTableOnDeleteResultsGroups, now;

        /**
         * Description measured results of the sample.
         */
        static const char *_sqlCreateTableResults =
            "create table if not exists results (                                       "
            "    id                         integer         primary key autoincrement,  "
            "    result_group_id            integer         not null,                   "
            "    parameter_id               integer         not null,                   "
            "                                                                           "
            "    measure_stamp              datetime        not null default (datetime(current_timestamp, 'localtime')), "
            "                                                                           "
            "    units                      varchar(16)     not null default '',        "
            "    flags                      varchar(64)     not null default '',        "
            "                                                                           "
            "    status                     varchar(64)     not null default '',        "
            "    comment                    varchar(64)     not null default '',        "
            "                                                                           "
            "    val                        varchar(512)    not null                    "
            ")                                                                          "
        ;
        session << _sqlCreateTableResults, now;

        static const char *_sqlCreateTableOnInsertResults =
            "create trigger if not exists [insert_results] after insert                 "
            "on [results]                                                               "
            "for each row                                                               "
            "begin                                                                      "
            "                                                                           "
            "insert or replace into stats_results(stamp, parameter_id, measure_type, count)  "
            "values (                                                                   "
            "    date(current_date, 'localtime'),                                       "
            "    new.parameter_id,                                                      "
            "    (select measure_type from results_groups where id = new.result_group_id), "
            "    coalesce((select count from stats_results                              "
            "       where stamp = date(current_date, 'localtime') and                   "
            "           parameter_id = new.parameter_id and                             "
            "           measure_type = (select measure_type from results_groups         "
            "                           where id = new.result_group_id)                 "
            "    ),0) + 1                                                               "
            ");                                                                         "
            "                                                                           "
            "end                                                                        "
        ;
        session << _sqlCreateTableOnInsertResults, now;

        /**
         * Description measured results of the sample.
         */
        static const char *_sqlCreateTableStatsResults =
            "create table if not exists stats_results (                                 "
            "    stamp                      datetime        not null,                   "
            "    parameter_id               integer         not null,                   "
            "    measure_type               integer         not null default 0,         "
            "    count                      integer         not null default 0          "
            ")                                                                          "
        ;
        session << _sqlCreateTableStatsResults, now;

        /**
         * Description measured results of the sample.
         */
        static const char *_sqlCreateUniqueIndexStatsResults =
            "create unique index if not exists uinx_stats_results on stats_results (stamp, parameter_id, measure_type)"
        ;

        session << _sqlCreateUniqueIndexStatsResults, now;
    }
    catch(...) {
        throw;
    }

}
