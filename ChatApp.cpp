#include <iostream>
#include <sqlite3.h>
#include <string>
#include "database.h"
using namespace std;

#define ERROR_NUMBER_OF_ARGUMENTS "FAIL ERROR_NUMBER_OF_ARGUMENTS"
#define NOT_SIGN_IN "FAIL NOT_SIGN_IN"
#define USER_NOT_EXIST "FAIL USER_NOT_EXIST"

int main(int argc, char **argv)
{
	sqlite3 *DB;
	string createTableSql =
		"DROP TABLE IF EXISTS USER;"
		"DROP TABLE IF EXISTS CHANNEL_USER;"
		"DROP TABLE IF EXISTS FRIEND;"
		"DROP TABLE IF EXISTS REQUEST;"

		"CREATE TABLE USER("
		"username text PRIMARY KEY	 NOT NULL, "
		"socket int NOT NULL, "
		"password text	 NOT NULL, "
		"isSignedIn		 int	 NOT NULL); "

		"CREATE TABLE CHANNEL_USER("
		"channel_name text not null,"
		"member_name text not null,"
		"PRIMARY KEY(channel_name, member_name),"
		"FOREIGN KEY(member_name) REFERENCES user(username));"

		"CREATE TABLE FRIEND("
		"owner text	 NOT NULL, "
		"theFriend text	 NOT NULL,"
		"PRIMARY KEY(owner, theFriend)"
		"); "

		"CREATE TABLE Request("
		"fromFriend text	 NOT NULL, "
		"toFriend	text	 NOT NULL, "
		"isAccepted	int	 NOT NULL, "
		"PRIMARY KEY(fromFriend, toFriend));";
	int exit = 0;
	exit = sqlite3_open("chatapp.db", &DB);
	exit = sqlite3_exec(DB, createTableSql.c_str(), NULL, 0, NULL);
	if (exit != SQLITE_OK)
	{
		std::cerr << "Error Create Table" << std::endl;
		return 0;
	}

	int i, maxi, maxfd, listenfd, connfd, sockfd;
	int nready, client[FD_SETSIZE];
	ssize_t ret;
	fd_set readfds, allset;
	char sendBuff[BUFF_SIZE], rcvBuff[BUFF_SIZE];
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;

	//Step 1: Construct a TCP socket to listen connection request
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{ /* calls socket() */
		perror("\nError: ");
		return 0;
	}

	//Step 2: Bind address to socket
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(54000);

	if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
	{ /* calls bind() */
		perror("\nError: ");
		return 0;
	}

	//Step 3: Listen request from client
	if (listen(listenfd, 20) == -1)
	{ /* calls listen() */
		perror("\nError: ");
		return 0;
	}

	maxfd = listenfd; /* initialize */
	maxi = -1;		  /* index into client[] array */
	for (i = 0; i < FD_SETSIZE; i++)
		client[i] = -1; /* -1 indicates available entry */
	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);

	//Step 4: Communicate with clients
	while (1)
	{
		readfds = allset; /* structure assignment */
		nready = select(maxfd + 1, &readfds, NULL, NULL, NULL);
		if (nready < 0)
		{
			perror("\nError: ");
			return 0;
		}

		if (FD_ISSET(listenfd, &readfds))
		{ /* new client connection */
			clilen = sizeof(cliaddr);
			if ((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0)
				perror("\nError: ");
			else
			{
				printf("You got a connection from %s\n", inet_ntoa(cliaddr.sin_addr)); /* prints client's IP */
				for (i = 0; i < FD_SETSIZE; i++)
					if (client[i] < 0)
					{
						client[i] = connfd; /* save descriptor */
						break;
					}
				if (i == FD_SETSIZE)
				{
					printf("\nToo many clients");
					close(connfd);
				}

				FD_SET(connfd, &allset); /* add new descriptor to set */
				if (connfd > maxfd)
					maxfd = connfd; /* for select */
				if (i > maxi)
					maxi = i; /* max index in client[] array */

				if (--nready <= 0)
					continue; /* no more readable descriptors */
			}
		}

		for (i = 0; i <= maxi; i++)
		{ /* check all clients for data */
			if ((sockfd = client[i]) < 0)
				continue;

			if (FD_ISSET(client[i], &readfds))
			{ // if connected and in fd_set

				ret = recv(client[i], rcvBuff, BUFF_SIZE, 0);

				if (ret <= 0)
				{
					FD_CLR(sockfd, &allset);
					close(sockfd);
					client[i] = -1;
				}

				else
				{
					rcvBuff[ret] = '\0';
					string buffString(rcvBuff); // convert buff to string type
					string notiString = "";
					vector<string> buffPiece = split(buffString, ' '); // turn buffString to [command, argu1, argu2, etc...]
					string currentUsername = findNick(sockfd);

					// SIGNUP: create account
					if (buffPiece[0] == "SIGNUP")
					{
						int found = -1;
						if (buffPiece.size() != 3)
						{
							echoToClient(ERROR_NUMBER_OF_ARGUMENTS, sockfd);
							break;
						}

						if (currentUsername != "nonexist")
						{
							echoToClient("SIGN_OUT_FIRST", sockfd);
							break;
						}

						string insertSql =
							"INSERT INTO user VALUES (" + quotesql(buffPiece[1]) + "," + to_string(sockfd) + "," + quotesql(buffPiece[2]) + ", 1);";

						int result = sqlite3_exec(DB, insertSql.c_str(), NULL, 0, NULL);

						if (result != SQLITE_OK)
						{
							echoToClient("FAIL USERNAME_ALREADY_EXIST", sockfd);
							break;
						}

						// echo success SIGNUP command to client
						echoToClient("SUCCESS", sockfd);
					}
					// SIGNIN: sign in
					else if (buffPiece[0] == "SIGNIN")
					{
						if (buffPiece.size() != 3)
						{
							echoToClient(ERROR_NUMBER_OF_ARGUMENTS, client[i]);
							break;
						}

						if (currentUsername != "nonexist")
						{
							echoToClient("SIGN_OUT_FIRST", client[i]);
							break;
						}

						string selectSql = "SELECT * FROM user WHERE username=" + quotesql(buffPiece[1]) + " AND password=" + quotesql(buffPiece[2]);

						if (hasResult(DB, selectSql) < 0) // user not exist
						{
							echoToClient("FAIL INCORRECT_USERNAME_OR_PASSWORD", client[i]);
							break;
						}

						selectSql = "SELECT * FROM user WHERE username=" + quotesql(buffPiece[1]) + " AND isSignedIn=0";
						if (hasResult(DB, selectSql) < 0)
						{
							echoToClient("FAIL USER_ALREADY_SIGN_IN", client[i]);
							break;
						}

						string updateSql = string("UPDATE user SET isSignedIn=1, socket=") + to_string(client[i]) + " WHERE username=" + quotesql(buffPiece[1]);
						sqlite3_exec(DB, updateSql.c_str(), NULL, 0, NULL);

						selectSql = "SELECT * FROM user;";
						userList.clear();
						sqlite3_exec(DB, selectSql.c_str(), callback, 0, NULL);

						echoToClient("SUCCESS", client[i]);
					}

					// UPDATE: update username and password		UPDATE [new username] [new password]
					else if (buffPiece[0] == "UPDATE")
					{
						if (buffPiece.size() != 3)
						{
							echoToClient(ERROR_NUMBER_OF_ARGUMENTS, client[i]);
							break;
						}

						if (currentUsername == "nonexist")
						{
							echoToClient(NOT_SIGN_IN, client[i]);
							break;
						}

						string updateSql = string("UPDATE user SET username=") + quotesql(buffPiece[1]) + ", password=" + quotesql(buffPiece[2]) + " WHERE username=" + quotesql(currentUsername)

										   + "; UPDATE channel_user SET member_name=" + quotesql(buffPiece[1]) + "WHERE member_name=" + quotesql(currentUsername)

										   + "; UPDATE friend SET owner=" + quotesql(buffPiece[1]) + "WHERE owner=" + quotesql(currentUsername)

										   + "; UPDATE friend SET theFriend=" + quotesql(buffPiece[1]) + "WHERE theFriend=" + quotesql(currentUsername)

										   + "; UPDATE Request SET fromFriend=" + quotesql(buffPiece[1]) + "WHERE fromFriend=" + quotesql(currentUsername)

										   + "; UPDATE Request SET toFriend=" + quotesql(buffPiece[1]) + "WHERE toFriend=" + quotesql(currentUsername);

						int result = sqlite3_exec(DB, updateSql.c_str(), NULL, 0, NULL);
						if (result != SQLITE_OK)
						{
							echoToClient("FAIL", client[i]);
							break;
						}
						echoToClient("SUCCESS", client[i]);
					}

					// SIGNOUT sign out
					else if (buffPiece[0] == "SIGNOUT")
					{
						if (buffPiece.size() != 1)
						{
							echoToClient(ERROR_NUMBER_OF_ARGUMENTS, client[i]);
							break;
						}

						if (currentUsername == "nonexist")
						{
							echoToClient(NOT_SIGN_IN, client[i]);
							break;
						}
						else
						{
							// set isSignedIn to false
							string updateSql = "UPDATE user SET isSignedIn=0 WHERE socket=" + to_string(client[i]);
							sqlite3_exec(DB, updateSql.c_str(), NULL, 0, NULL);
							echoToClient("User sign out", client[i]);

							/*string selectSql = "SELECT * FROM user;";
							userList.clear();
							sqlite3_exec(DB, selectSql.c_str(), callback, 0, NULL);*/
						}
					}

					// LIST_FRIEND list all friend
					else if (buffPiece[0] == "LIST_FRIEND")
					{
						if (buffPiece.size() != 1)
						{
							echoToClient(ERROR_NUMBER_OF_ARGUMENTS, client[i]);
							break;
						}

						if (currentUsername == "nonexist")
						{
							echoToClient(NOT_SIGN_IN, client[i]);
							break;
						}
						string selectSql = "SELECT theFriend FROM friend WHERE owner=" + quotesql(currentUsername);
						globalVar.clear();
						sqlite3_exec(DB, selectSql.c_str(), resultCallback, 0, NULL);

						string response = "SUCCESS";
						for (auto &a : globalVar)
						{
							response += " ";
							response += a;
						}
						echoToClient(response, client[i]);
					}

					// LIST_ACTIVE_FRIEND list all active friend
					else if (buffPiece[0] == "LIST_FRIEND")
					{
						if (buffPiece.size() != 1)
						{
							echoToClient(ERROR_NUMBER_OF_ARGUMENTS, client[i]);
							break;
						}

						if (currentUsername == "nonexist")
						{
							echoToClient(NOT_SIGN_IN, client[i]);
							break;
						}
						string selectSql = "SELECT theFriend FROM friend WHERE owner=" + quotesql(currentUsername) + " AND theFriend IN (SELECT username FROM user WHERE isSignedIn=1)";
						globalVar.clear();
						sqlite3_exec(DB, selectSql.c_str(), resultCallback, 0, NULL);

						string response = "SUCCESS";
						for (auto &a : globalVar)
						{
							response += " ";
							response += a;
						}
						echoToClient(response, client[i]);
					}

					// SEARCH_FRIEND search friend
					else if (buffPiece[0] == "SEARCH_FRIEND")
					{
						if (buffPiece.size() != 2)
						{
							echoToClient(ERROR_NUMBER_OF_ARGUMENTS, client[i]);
							break;
						}

						if (currentUsername == "nonexist")
						{
							echoToClient(NOT_SIGN_IN, client[i]);
							break;
						}
						string selectSql = "SELECT theFriend FROM friend WHERE owner=" + quotesql(currentUsername) + " AND theFriend LIKE '%" + quotesql(buffPiece[1]) + "%'";
						globalVar.clear();
						sqlite3_exec(DB, selectSql.c_str(), resultCallback, 0, NULL);

						string response = "SUCCESS";
						for (auto &a : globalVar)
						{
							response += " ";
							response += a;
						}
						echoToClient(response, client[i]);
					}

					// FRIENDMSG: send message to an user
					else if (buffPiece.at(0) == "FRIENDMSG")
					{
						if (buffPiece.size() < 3)
						{
							echoToClient(ERROR_NUMBER_OF_ARGUMENTS, client[i]);
							break;
						}

						if (currentUsername == "nonexist")
						{
							echoToClient(NOT_SIGN_IN, client[i]);
							break;
						}

						string msg;
						for (int j = 2; j < buffPiece.size(); j++)
							msg = msg + buffPiece[j] + " ";

						string selectSql = "SELECT socket";
					}

					// ADDFRIEND add friend
					else if (buffPiece[0] == "ADDFRIEND")
					{
						if (buffPiece.size() != 2)
						{
							echoToClient(ERROR_NUMBER_OF_ARGUMENTS, client[i]);
							break;
						}

						if (currentUsername == "nonexist")
						{
							echoToClient(NOT_SIGN_IN, client[i]);
							break;
						}

						globalVar.clear();
						string selectSql = "SELECT socket FROM user WHERE username=" + quotesql(buffPiece[1]) + " UNION SELECT username FROM user WHERE socket=" + to_string(client[i]);
						sqlite3_exec(DB, selectSql.c_str(), resultCallback, 0, NULL);

						if (globalVar.size() < 2)
						{
							echoToClient(USER_NOT_EXIST, client[i]);
							break;
						}

						// check if can add friend
						// 3 conditions: user exist + user is not friend + user is not added friend
						selectSql = "SELECT * FROM user WHERE username NOT IN (SELECT theFriend FROM friend WHERE owner=" + quotesql(buffPiece[1]) + ") AND username NOT IN (SELECT toFriend FROM Request WHERE fromFriend=" + quotesql(globalVar[1]) + ")";

						if (hasResult(DB, selectSql) < 0)
						{
							echoToClient("FAIL ALREADY_FRIEND", client[i]);
							break;
						}

						string insertSql = string("INSERT INTO Request values((SELECT username FROM user WHERE socket=") +
										   to_string(client[i]) + ")," +
										   quotesql(buffPiece[1]) + ", 0)";

						int result = sqlite3_exec(DB, insertSql.c_str(), NULL, 0, NULL);

						if (result != SQLITE_OK)
						{
							echoToClient("FAIL ALREADY_ADD_FRIEND", client[i]);
							break;
						}
						echoToClient("SUCCESS", client[i]);

						// maybe check if the friend is online then echo
						echoToClient("SUCCESS REQUEST_FROM " + globalVar[1], stoi(globalVar[0]));
					}

					// ACCEPTFRIEND accept friend
					else if (buffPiece[0] == "ACCEPTFRIEND")
					{
						if (buffPiece.size() != 2)
						{
							echoToClient(ERROR_NUMBER_OF_ARGUMENTS, client[i]);
							break;
						}

						if (currentUsername == "nonexist")
						{
							echoToClient(NOT_SIGN_IN, client[i]);
							break;
						}
						// get currentUsername and get socket of buffPiece[1]

						globalVar.clear();
						string selectSql = "SELECT socket FROM user WHERE username=" + quotesql(buffPiece[1]) + " UNION SELECT username FROM user WHERE socket=" + to_string(client[i]);
						sqlite3_exec(DB, selectSql.c_str(), resultCallback, 0, NULL);

						if (globalVar.size() < 2)
						{
							echoToClient(USER_NOT_EXIST, client[i]);
							break;
						}

						//update friend and request table
						string updateSql = "UPDATE Request SET  isAccepted = 1 WHERE fromFriend=" + quotesql(buffPiece[1]) + " AND toFriend=" + quotesql(globalVar[1]) +
										   ";INSERT INTO friend VALUES(" + quotesql(globalVar[1]) + ", " + quotesql(buffPiece[1]) + ");"
																																	"INSERT INTO friend VALUES(" +
										   quotesql(buffPiece[1]) + ", " + quotesql(globalVar[1]) + ");";
						int result = sqlite3_exec(DB, updateSql.c_str(), NULL, 0, NULL);

						if (result != SQLITE_OK)
						{
							echoToClient("FAIL NO_REQUEST", client[i]);
							break;
						}
						echoToClient("SUCCESS", client[i]);
						echoToClient("SUCCESS ACCEPT_FROM " + globalVar[1], stoi(globalVar[0]));
					}

					// JOIN: join channel, echo notification
					else if (buffPiece[0] == "JOIN")
					{
						if (buffPiece.size() != 2)
						{
							notiString = "Error number of arguments! ";
							echoToClient(notiString, client[i]);
							break;
						}

						if (currentUsername == "nonexist")
						{
							echoToClient(NOT_SIGN_IN, client[i]);
							break;
						}

						string insertSql = "INSERT INTO channel_user VALUES((SELECT channel_name FROM channel_user WHERE channel_name=" + quotesql(buffPiece[1]) + " LIMIT 1)," + quotesql(currentUsername) + ")";

						int result = sqlite3_exec(DB, insertSql.c_str(), NULL, 0, NULL);

						if (result != SQLITE_OK)
						{
							echoToClient("FAIL", client[i]);
							break;
						}
						echoToClient("SUCCESS", client[i]);
						echoToClient("SUCCESS JOIN_FROM ", client[i]);
					}

					// CREATE: create channel
					else if (buffPiece[0] == "CREATE")
					{
						if (buffPiece.size() != 2)
						{
							notiString = "Error number of arguments! ";
							echoToClient(notiString, client[i]);
							break;
						}

						if (currentUsername == "nonexist")
						{
							echoToClient(NOT_SIGN_IN, client[i]);
							break;
						}

						string insertSql = "INSERT INTO channel_user VALUES(" + quotesql(buffPiece[1]) + "," + quotesql(currentUsername) + ")";

						int result = sqlite3_exec(DB, insertSql.c_str(), NULL, 0, NULL);

						if (result != SQLITE_OK)
						{
							echoToClient("FAIL GROUP_EXISTS", client[i]);
							break;
						}
						echoToClient("SUCCESS", client[i]);
					}

					// QUIT: get out of a channel
					else if (buffPiece.at(0) == "QUIT")
					{
						if (buffPiece.size() != 2)
						{
							echoToClient("Error number of arguments! ", client[i]);
							break;
						}

						if (currentUsername == "nonexist")
						{
							echoToClient(NOT_SIGN_IN, client[i]);
							break;
						}

						string deleteSql = "DELETE FROM channel_user WHERE member_name=" + quotesql(currentUsername);
						int result = sqlite3_exec(DB, deleteSql.c_str(), NULL, 0, NULL);

						if (result != SQLITE_OK)
						{
							echoToClient("FAIL", client[i]);
							break;
						}
						echoToClient("SUCCESS ", client[i]);
					}

					//// LISTGROUP: show all channel and number of members per channel
					//else if (buffPiece.at(0) == "LISTGROUP")
					//{
					//	if (buffPiece.size() != 1)
					//	{
					//		echoToClient("Missing/error argument! ", client[i]);
					//		break;
					//	}

					//	if (currentUsername == "nonexist" || !userList[findUserBySocket(client[i])].isSignedIn)
					//	{
					//		echoToClient("Please log in first ", client[i]);
					//		break;
					//	}

					//	for (auto& chan : channelList)
					//		echoToClient("Channel " + chan.name + ": " + to_string(chan.member.size()) + " members", client[i]);
					//}

					//// LISTFRIEND: show all friends
					//else if (buffPiece.at(0) == "LISTFRIEND")
					//{
					//	if (buffPiece.size() != 1)
					//	{
					//		echoToClient("Missing/error argument! ", client[i]);
					//		break;
					//	}

					//	if (currentUsername == "nonexist" || !userList[findUserBySocket(client[i])].isSignedIn)
					//	{
					//		echoToClient("Please log in first ", client[i]);
					//		break;
					//	}

					//	for (auto& u : userList[findUserBySocket(client[i])].friends)
					//		echoToClient(u.nickname + "\n", client[i]);
					//}

					//// GROUPMSG: send message to a group
					//else if (buffPiece.at(0) == "GROUPMSG")
					//{
					//	bool found = false;

					//	if (buffPiece.size() < 3)
					//	{
					//		notiString = "Missing argument! ";
					//		echoToClient(notiString, client[i]);
					//		break;
					//	}

					//	if (currentUsername == "nonexist" || !userList[findUserBySocket(client[i])].isSignedIn)
					//	{
					//		notiString = "Please sign in first ";
					//		echoToClient(notiString, client[i]);
					//		break;
					//	}

					//	string msg;
					//	for (int j = 2; j < buffPiece.size(); j++)
					//		msg = msg + buffPiece[j] + " ";

					//	for (auto& a : channelList)
					//		if (a.name == buffPiece[1])
					//		{
					//			notiString = currentUsername + " to channel " + buffPiece[1] + " : " + msg;
					//			for (auto& u : a.member)
					//				if (u.isSignedIn)
					//					echoToClient(notiString, u.userSock);
					//			found = true;
					//			break;
					//		}

					//	if (!found)
					//		echoToClient("No channel has name " + buffPiece[1], client[i]);
					//}

					//// WHO: show info about user or channel(all members in a channel)
					//else if (buffPiece.at(0) == "WHO")
					//{
					//	bool found = false;

					//	if (buffPiece.size() != 2)
					//	{
					//		notiString = "Missing/error argument! ";
					//		echoToClient(notiString, client[i]);
					//		break;
					//	}

					//	if (currentUsername == "nonexist" || !userList[findUserBySocket(client[i])].isSignedIn)
					//	{
					//		notiString = "Please log in first ";
					//		echoToClient(notiString, client[i]);
					//		break;
					//	}

					//	for (auto& a : channelList)
					//		if (a.name == buffPiece[1])
					//		{
					//			notiString = "All member of channel " + buffPiece[1] + ":";
					//			echoToClient(notiString, client[i]);

					//			for (auto& u : a.member)
					//			{
					//				notiString = u.nickname + "  ";
					//				echoToClient(notiString, client[i]);
					//			}

					//			found = true;
					//			break;
					//		}
					//	if (!found)
					//	{
					//		notiString = "No channel found with name " + buffPiece[1];
					//		echoToClient(notiString, client[i]);
					//	}
					//}
					if (ret <= 0)
					{
						FD_CLR(sockfd, &allset);
						close(sockfd);
						client[i] = -1;
					}
				}
			}

			if (--nready <= 0)
				continue; //no more event
		}
	}
	// close listening socket
	close(listenfd);

	sqlite3_close(DB);
	return (0);
}
