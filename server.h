#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
using namespace std;
#define BUFF_SIZE 1024

typedef struct user
{
	string nickname;
	int userSock;
	string password;
	int isSignedIn;
} user;

extern vector<user> userList;

typedef struct channel
{
	string name;
	vector<string> member;
} channel;

extern vector<channel> channelList;
extern vector<string> globalVar;

vector<string> split(const string &s, char delim);
int findSocket(const string &nick);	   // find socket based on nickname
string findNick(const int &sock);	   // find nickname based on socket
int findUserBySocket(const int &sock); // find user based on socket
int findUserByName(const string &username);
int findChannelByName(const string &name);
void echoToClient(const string &notiString, const int &soc);
string quotesql(const string &s);
