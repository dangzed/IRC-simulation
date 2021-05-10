#include "sqlite3.h"
#include "server.h"


int callback(void* data, int argc, char** argv, char** azColName);
int resultCallback(void* data, int argc, char** argv, char** azColName);
int hasResult(sqlite3* database, const string& query);
