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
#define BUFF_SIZE 210

typedef struct user {
	string nickname;
	SOCKET userSock;
}user;

vector<user> userList;

typedef struct channel {
	string name;
	vector<user> member;
}channel;

vector<channel> channelList;

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
	for (user u : userList)
		if (u.nickname == nick)
			return u.userSock;
	return -1;
}

string findNick(const SOCKET& sock)
{
	for (user u : userList)
		if (u.userSock == sock)
			return u.nickname;
	return "nonexist";
}


int findChannelBySocket(const SOCKET& sock)
{
	for (int i = 0; i < channelList.size(); i++)
	{
		for (auto a : channelList[i].member)
			if (a.userSock == sock)
				return i;
	}
	return -1;
}

int findChannelByName(const string& name)
{
	for (int i = 0; i < channelList.size(); i++)
		if (channelList[i].name == name)
			return i;
	return -1;
}

void deleteUserOutOfChannel(int index, const SOCKET& sock)
{
	for (int i = 0; i < channelList[index].member.size(); i++)
		if (channelList[index].member[i].userSock == sock)
		{
			channelList[index].member.erase(channelList[index].member.begin() + i);
			return;
		}
}

void echoToClient(const string& notiString, const SOCKET& soc)
{
	char notiBuff[210] = "";
	strcpy(notiBuff, notiString.c_str());
	send(soc, notiBuff, notiString.size(), 0);
}

void main() {

	// initialize users list using vector

	// Initialize winsock
	WSAData wsData;
	WORD ver = MAKEWORD(2, 2);
	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0) {
		cerr << "\nCant initialize winsock  ";
		return;
	}

	// Create a socket
	SOCKET listenSock = socket(AF_INET, SOCK_STREAM, 0);

	// Bind the socket to ip address and port
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(54000);
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");	// can use inet_pton too...
	bind(listenSock, (sockaddr*)&serverAddr, sizeof(serverAddr));

	// Tell Winsock the socket is for listening
	listen(listenSock, SOMAXCONN);

	// Assign initial value for the array of connection socket
	SOCKET client[FD_SETSIZE], connSock;
	fd_set readfds, initfds;	//use initfds to initiate readfds at the begining of every loop step
	sockaddr_in clientAddr;
	int nEvents, ret, clientAddrLen = sizeof(clientAddr);
	char buff[BUFF_SIZE];

	for (size_t i = 0; i < FD_SETSIZE; i++)
	{
		client[i] = 0;
	}
	FD_ZERO(&initfds);	//Assign initial value for the fd_set
	FD_SET(listenSock, &initfds);

	// communicate with clients
	while (true) {

		//Add listenSock to readfds
		readfds = initfds;
		nEvents = select(0, &readfds, 0, 0, 0);

		if (nEvents < 0) {
			cout << "\nError! Cannot poll socket " << WSAGetLastError() << endl;
			return;
		}

		// new client connection
		if (FD_ISSET(listenSock, &readfds)) {	// if listenSock is in fd_set

			if ((connSock = accept(listenSock, (sockaddr*)&clientAddr, &clientAddrLen)) < 0) {
				cout << "\nError! Cannot accept new connection: %d", WSAGetLastError();
				break;
			}	// do accept function

			else {
				cout << "\nConnection from [" << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << "]" << endl;

				int i;

				for (i = 0; i < FD_SETSIZE; i++)

					if (client[i] == 0) {
						client[i] = connSock;
						FD_SET(client[i], &initfds);
						break;
					}

				if (i == FD_SETSIZE) {
					cout << "\nToo many clients.";
					closesocket(connSock);
				}

				if (--nEvents == 0)
					continue; //no more event
			}
		}
		// end of connection establishment phase


		//receive data from clients
		for (int i = 0; i < FD_SETSIZE; i++) {

			if (client[i] == 0)	// if not connected skip
				continue;

			if (FD_ISSET(client[i], &readfds)) {	// if connected and in fd_set 

				ret = recv(client[i], buff, BUFF_SIZE, 0);

				if (ret <= 0)
				{	 // if client disconnected 
					FD_CLR(client[i], &initfds);	//clear it out of fd_set
					closesocket(client[i]);			// close socket
					cout << "Close socket " << client[i] << endl;
					client[i] = 0;					// reset in client array
				}

				else
				{
					buff[ret] = '\0';
					string buffString(buff);		// convert buff to string type
					string notiString;
					vector<string> buffPiece = split(buffString, ' ');	// turn buffString to command, argu1, argu2, etc...

					// NICK: check if nickname in initial list, echo error if nickname already existed, otherwise add to nickname list
					if (buffPiece[0] == "NICK")
					{
						int found = -1;
						if (buffPiece.size() != 2)
						{
							notiString = "Missing/error argument! ";
							echoToClient(notiString, client[i]);
							break;
						}

						for (auto u : userList)
							if (u.nickname == buffPiece[1])
								found = 1;

						if (found > 0)		// if found in user list
						{
							notiString = "Error: Nickname already existed";
							echoToClient(notiString, client[i]);
						}

						else
						{
							// if user want to change nickname
							if (findNick(client[i]) != "nonexist")
							{
								// change data in userList
								for (int j = 0; j < userList.size(); j++)
									if (userList[j].userSock == client[i])
									{
										userList[j].nickname = buffPiece[1];
										break;
									}
								notiString = "Change nickname successfully, now you are " + buffPiece[1];
								echoToClient(notiString, client[i]);

								// change data in current channel (if user being in any channel)
								int index = findChannelBySocket(client[i]);
								if (index >= 0)
									for (int j = 0; j < channelList[index].member.size(); j++)
										if (channelList[index].member[j].userSock == client[i])
										{
											channelList[index].member[j].nickname = buffPiece[1];
											break;
										}
							}

							// if new user want to create nickname
							else
							{
								user tempUser;
								tempUser.nickname = buffPiece[1];
								tempUser.userSock = client[i];
								userList.push_back(tempUser);		// push to user list
								// echo success NICK command to client
								notiString = "Set nickname successfully, hello " + buffPiece[1];
								echoToClient(notiString, client[i]);
							}
						}
					}

					// JOIN: join channel, echo notification 
					else if (buffPiece[0] == "JOIN")
					{
						if (buffPiece.size() != 2)
						{
							notiString = "Missing/error argument! ";
							echoToClient(notiString, client[i]);
							break;
						}

						if (findNick(client[i]) == "nonexist")
						{
							notiString = "You dont have a nickname! Please set it now ";
							echoToClient(notiString, client[i]);
							break;
						}

						// find current channel of user
						int index = findChannelBySocket(client[i]);
						int found = findChannelByName(buffPiece[1]);

						// if user being in 1 channel => clear it out of that channel 
						if (index >= 0)
						{
							if (index == found)		// if argument channel is current channel i
							{
								notiString = "You are already in channel " + channelList[found].name;
								echoToClient(notiString, client[i]);
								break;
							}

							else
							{
								// else clear it out of current channel
								deleteUserOutOfChannel(index, client[i]);
								// echo noti to all client on current channel
								notiString = findNick(client[i]) + " left " + channelList[index].name;
								for (user u : channelList[index].member)
									echoToClient(notiString, u.userSock);
							}
						}

						if (found >= 0)		// channel is in channelList
						{
							// join argument channel, echo to all client in argument channel
							for (user u : userList)
								if (u.userSock == client[i])
								{
									channelList[found].member.push_back(u);
									notiString = u.nickname + " join channel " + channelList[found].name;
									// echo noti to all client in new channel
									for (user us : channelList[found].member)
										echoToClient(notiString, us.userSock);
									break;
								}
						}

						else 	// if channel not existed create one
						{
							for (user u : userList)
								if (u.userSock == client[i])
								{
									channel tempChannel;
									tempChannel.name = buffPiece[1];
									tempChannel.member.push_back(u);
									channelList.push_back(tempChannel);
									notiString = u.nickname + " create new channel: " + tempChannel.name;
									echoToClient(notiString, client[i]);
									break;
								}
						}
					}

					else if (buffPiece[0] == "QUIT")
					{
						// if nickname not set
						if (findNick(client[i]) == "nonexist")
							break;

						// if in any channel clear it out
						int found = findChannelBySocket(client[i]);
						if (found < 0)			// if not in any channel break
							break; 
						deleteUserOutOfChannel(found, client[i]);

						notiString = findNick(client[i]) + " end session, quitting..." ;
						for (user u : channelList[found].member)
							echoToClient(notiString, u.userSock);
					}

					// PART: out of current channel 
					else if (buffPiece.at(0) == "PART")
					{
					if (buffPiece.size() != 1)
					{
						notiString = "Missing/error argument! ";
						echoToClient(notiString, client[i]);
						break;
					}

						if (findNick(client[i]) == "nonexist")
						{
							notiString = "You dont have a nickname! Please set it now ";
							echoToClient(notiString, client[i]);
							break;
						}

						int found = findChannelBySocket(client[i]);
						if (found < 0)			// if not in any channel break
							break;

						deleteUserOutOfChannel(found, client[i]);

						notiString = findNick(client[i]) + " left channel " + channelList[found].name;
						for (user u : channelList[found].member)
							echoToClient(notiString, u.userSock);
					}

					// LIST: show all channel and number of members per channel
					else if (buffPiece.at(0) == "LIST")
					{
						if (buffPiece.size() != 1)
						{
							notiString = "Missing/error argument! ";
							echoToClient(notiString, client[i]);
							break;
						}

						for (auto chan : channelList)
						{
							notiString = "Channel " + chan.name + ": " + to_string(chan.member.size()) + " members";
							echoToClient(notiString, client[i]);
						}
					}

					// PRIVMSG: send message to a channel or an user
					else if (buffPiece.at(0) == "PRIVMSG")
					{
						bool found = false;

						if (buffPiece.size() < 3)
						{
							notiString = "Missing argument! ";
							echoToClient(notiString, client[i]);
							break;
						}

						if (findNick(client[i]) == "nonexist")
						{
							notiString = "You dont have a nickname! Please set it now ";
							echoToClient(notiString, client[i]);
							break;
						}

						string msg;
						for (int j = 2; j < buffPiece.size(); j++)
							msg = msg + buffPiece[j] + " ";

						// find out user want to send private msg to channel or another user
						// if channel
						for (auto a : channelList)
							if (a.name == buffPiece[1])
							{
								notiString = findNick(client[i]) + " to channel " + buffPiece[1] + " : " + msg;
								for (user u : a.member)
									echoToClient(notiString, u.userSock);
								found = true;
								break;
							}
						// if user
						for (auto a : userList)
							if (a.nickname == buffPiece[1])
							{
								notiString = findNick(client[i]) + ">> " + msg;
								echoToClient(notiString, a.userSock);
								found = true;
								break;
							}
						if (!found)
						{
							notiString = "No channel or user has name " + buffPiece[1];
							echoToClient(notiString, client[i]);
						}
					}

					// WHO: show info about user or channel(all members in a channel)
					else if (buffPiece.at(0) == "WHO")
					{
						bool found = false;

						if (buffPiece.size() != 2)
						{
							notiString = "Missing/error argument! ";
							echoToClient(notiString, client[i]);
							break;
						}

						for (auto a : channelList)
							if (a.name == buffPiece[1])
							{
								notiString = "All member of channel " + buffPiece[1] + ":";
								echoToClient(notiString, client[i]);

								for (user u : a.member)
								{
									notiString = u.nickname + "  ";
									echoToClient(notiString, client[i]);
								}

								found = true;
								break;
							}
						if (!found)
						{
							notiString = "No channel found with name " + buffPiece[1];
							echoToClient(notiString, client[i]);
						}
					}

					else if (buffPiece.at(0) == "HELP")
					{
						if (buffPiece.size() != 1)
						{
							notiString = "Missing/error argument! ";
							echoToClient(notiString, client[i]);
							break;
						}

						notiString = "NICK, JOIN, PART, PRIVMSG, LIST, WHO ";
						echoToClient(notiString, client[i]);
					}

					else
					{
						notiString = "ERR_UNKNOWNCOMMAND";
						echoToClient(notiString, client[i]);
					}

				}
			}

			if (--nEvents <= 0)
				continue; //no more event
		}
	}

	// close listening socket
	closesocket(listenSock);

	// cleanup winsock
	WSACleanup();

	return;
}