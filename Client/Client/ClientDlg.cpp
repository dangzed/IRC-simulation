
// ClientDlg.cpp : implementation file

#include "stdafx.h"
#include "Client.h"
#include "ClientDlg.h"
#include "ClientCon.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD) 
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()



CClientDlg::CClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CClientDlg::IDD, pParent),
	m_pClient(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_ChatBox);
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON2, &CClientDlg::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON4, &CClientDlg::OnBnClickedButtonSend)
	ON_BN_CLICKED(IDC_BUTTON3, &CClientDlg::OnBnClickedButtonClose)
END_MESSAGE_MAP()


// CClientDlg message handlers

BOOL CClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	CWnd* pWnd = GetDlgItem(IDC_BUTTON3);
	pWnd->ShowWindow(SW_HIDE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CClientDlg::OnBnClickedButtonConnect()
{
	// TODO: Add your control notification handler code here
	cTh = AfxBeginThread(StaticThreadFunc, this);
	
	m_Thread_handle = cTh->m_hThread;

	// sau khi đăng nhập, ẩn connect và hiện close
	CWnd* pWnd = GetDlgItem(IDC_BUTTON3);
	pWnd->ShowWindow(SW_SHOW);
	pWnd = GetDlgItem(IDC_BUTTON2);
	pWnd->ShowWindow(SW_HIDE);
}


void CClientDlg::OnBnClickedButtonSend()
{
	// TODO: Add your control notification handler code here 
	CString sTextData;
	GetDlgItemText(IDC_EDIT7, sTextData); 
	
	CT2CA CStringToAscii(sTextData);

     // construct a std::string using the LPCSTR input
    std::string sResultedString (CStringToAscii);
	if(m_pClient != NULL)
		m_pClient->SendData(sResultedString);

	// chuyển vùng nhập tin nhắn về rỗng
	CWnd* pWnd = GetDlgItem(IDC_EDIT7); 
	pWnd->SetWindowText(_T(""));
}

void CClientDlg::OnBnClickedButtonClose()
{
	// TODO: Add your control notification handler code here
	m_ChatBox.ResetContent();
	UpdateData();
	string quit = "QUIT";
	if (m_pClient != NULL)
		m_pClient->SendData(quit);
	delete m_pClient;

	// sau khi đăng xuất, ẩn log out và hiện login
	CWnd* pWnd = GetDlgItem(IDC_BUTTON3);
	pWnd->ShowWindow(SW_HIDE);
	pWnd = GetDlgItem(IDC_BUTTON2);
	pWnd->ShowWindow(SW_SHOW);
}

void CClientDlg::ShowServerInfo(string sValue)	// gửi thông điệp lên cửa sổ chat
{
	CString strLine(sValue.c_str());
	m_ChatBox.AddString(strLine);
}

UINT __cdecl CClientDlg::StaticThreadFunc(LPVOID pParam)
{
	CClientDlg *pYourClass = reinterpret_cast<CClientDlg*>(pParam);
    UINT retCode = pYourClass->ThreadFunc();

    return retCode;
}

UINT CClientDlg::ThreadFunc()
{ 
    // Do your thing, this thread now has access to all the classes member variables
	
	m_pClient = new ClientCon(this);

	CString username;
	GetDlgItemText(IDC_EDIT6, username);
     
	CT2CA CStringToAscii(username);

	std::string sResultedString (CStringToAscii);

	m_pClient->StartConnect("127.0.0.1", 54000);
	return 0;
}

