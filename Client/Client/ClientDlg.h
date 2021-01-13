

#pragma once
#include "ClientCon.h"
#include <Windows.h>
#include "resource.h"

// CClientDlg dialog
class CClientDlg : public CDialogEx
{
// Construction
public:
	CClientDlg(CWnd* pParent = NULL);	// standard constructor
	void ShowServerInfo(string sValue);

// Dialog Data
enum { IDD = IDD_CLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonConnect();
	afx_msg void OnBnClickedButtonSend();

	ClientCon *m_pClient;
	static UINT __cdecl StaticThreadFunc(LPVOID pParam);
    UINT ThreadFunc();

private:
	HANDLE m_Thread_handle;
	CWinThread *cTh;

public:
	afx_msg void OnBnClickedButtonClose();
	CListBox m_ChatBox;
};
