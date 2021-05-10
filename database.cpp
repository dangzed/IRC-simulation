#include "database.h"

int callback(void* data, int argc, char** argv, char** azColName)
{
	int i;
	user tempUser;
	for (i = 0; i < argc; i++) {
		if (!strcmp(azColName[i], "username"))
			tempUser.nickname = argv[i];
		else if (!strcmp(azColName[i], "socket"))
			tempUser.userSock = stoi(argv[i]);
		else if (!strcmp(azColName[i], "password"))
			tempUser.password = argv[i];
		else if (!strcmp(azColName[i], "isSignedIn"))
			tempUser.isSignedIn = stoi(argv[i]);
	}
	
	userList.push_back(tempUser);

	printf("\n");
	return 0;
}

int hasResult(sqlite3* database, const string& query)
{
	sqlite3_stmt* stmt;
	int result;


	int rc = sqlite3_prepare_v2(database, query.c_str(), -1, &stmt, NULL);
	if (rc != SQLITE_OK) {
		printf("error: %s\n", sqlite3_errmsg(database));
		return -1;
	}

	rc = sqlite3_step(stmt);
	if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
		printf("error: %s\n", sqlite3_errmsg(database));
		sqlite3_finalize(stmt);
		return -1;
	}

	if (rc == SQLITE_DONE) // no result
		result = -1;
	else if (sqlite3_column_type(stmt, 0) == SQLITE_NULL) // result is NULL
		result = -1;
	else { // some valid result
		result = sqlite3_column_int(stmt, 0);
	}

	sqlite3_finalize(stmt);

	return result;
}

int resultCallback(void* data, int argc, char** argv, char** azColName)
{
	int i;
	for (i = 0; i < argc; i++) 
		globalVar.push_back(argv[i]);
	return 0;
}