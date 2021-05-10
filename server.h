#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WS2tcpip.h>
#include <process.h>
#include <string>
#include <vector>
#include <sstream>
#pragma comment (lib, "ws2_32.lib")
using namespace std;
#define BUFF_SIZE 1024

typedef struct user {
	string nickname;
	SOCKET userSock;
	string password;
	int isSignedIn;
}user;

extern vector<user> userList; 

typedef struct channel {
	string name;
	vector<string> member;
}channel;

extern vector<channel> channelList;
extern vector<string> globalVar;

vector<string> split(const string& s, char delim);
SOCKET findSocket(const string& nick);				// find socket based on nickname
string findNick(const SOCKET& sock);				// find nickname based on socket
int findUserBySocket(const SOCKET& sock);					// find user based on socket
int findUserByName(const string& username);
//int findChannelByUsername(const string& username);
int findChannelByName(const string& name);
//void deleteUserOutOfChannel(int index, const SOCKET& sock);
void echoToClient(const string& notiString, const SOCKET& soc);
string quotesql(const string& s);
