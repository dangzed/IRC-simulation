
#pragma once

#include "string.h"
#include "string"
#include<winsock2.h>
using namespace std;
class CClientDlg;

class ClientCon
{
public:
	ClientCon(CClientDlg *dlg);
	~ClientCon(void);
	void StartConnect(string sAddress, int iPort);
	void SendData(string sMessage);

	WSADATA wsa;
    SOCKET s;
	CClientDlg *m_pClient;
	string m_pUser; 
};

