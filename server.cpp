#include "server.h"

vector<user> userList;
vector<channel> channelList;
vector<string> globalVar;


vector<string> split(const string& s, char delim) {
	vector<string> result;
	stringstream ss(s);
	string item;

	while (getline(ss, item, delim)) {
		result.push_back(item);
	}

	return result;
}

SOCKET findSocket(const string& nick)
{
	for (auto &u : userList)
		if (u.nickname == nick)
			return u.userSock;
	return -1;
}

string findNick(const SOCKET& sock)
{
	for (auto& u : userList)
		if (u.userSock == sock)
			return u.nickname;
	return "nonexist";
}


int findChannelByName(const string& name)
{
	for (int i = 0; i < channelList.size(); i++)
		if (channelList[i].name == name)
			return i;
	return -1;
}

void echoToClient(const string& notiString, const SOCKET& soc)
{
	char notiBuff[1024] = "";
	strcpy(notiBuff, notiString.c_str());
	send(soc, notiBuff, notiString.size(), 0);
}

int findUserBySocket(const SOCKET& sock) {
	for (int i = 0; i < userList.size(); i++)
		if (userList[i].userSock == sock)
			return i;

	return -1;
}

int findUserByName(const string& username) {
	for (int i = 0; i < userList.size(); i++)
		if (userList[i].nickname == username)
			return i;

	return -1;
}

string quotesql(const string& s) {
	return string("'") + s + string("'");
}

