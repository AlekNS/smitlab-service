
#ifndef DB_CREATE_H
#define DB_CREATE_H


namespace Poco { namespace Data { class Session; } }


namespace common {


using Poco::Data::Session;


void DBInit(Session &session, bool useExtends);
void DBCreate(Session &session);
void DBVacuum(Session &session);


}


#endif
