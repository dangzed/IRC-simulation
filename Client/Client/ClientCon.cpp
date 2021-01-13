
#include "StdAfx.h"
#include "ClientCon.h"
#include<stdio.h>

#include <string.h>
#include "ClientDlg.h"
 
#pragma comment(lib,"ws2_32.lib") 

ClientCon::ClientCon(CClientDlg *dlg)
{
	m_pClient = dlg;
}


ClientCon::~ClientCon(void)
{
	closesocket(s);
	WSACleanup();
}

void ClientCon::StartConnect(string sAddress, int iPort)
{
    sockaddr_in server;
    char server_reply[210]; 
    int recv_size;
	//m_pUser = sUsername;

    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
        return;
     
    //Create a socket
    s = socket(AF_INET, SOCK_STREAM, 0);
 
	server.sin_addr.s_addr = inet_addr(sAddress.c_str());
    server.sin_family = AF_INET;
    server.sin_port = htons( iPort );
 
    //Connect to remote server
    if (connect(s , (sockaddr *)&server , sizeof(server)) < 0)
        return;
     
    //Receive a reply from the server
    while((recv_size = recv(s , server_reply , 210 , 0)) != SOCKET_ERROR)
    { 
		//Add a NULL terminating character to make it a proper string before printing
		server_reply[recv_size] = '\0';

		string sTempMsg = string(server_reply);
		m_pClient->ShowServerInfo(sTempMsg);
    }
    
}

void ClientCon::SendData(string sMessage)
{
	if( send(s , sMessage.c_str(), sMessage.size() , 0) < 0)
        return;
}