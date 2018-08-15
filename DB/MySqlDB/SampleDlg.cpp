
// SampleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Sample.h"
#include "SampleDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
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

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{

}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CSampleDlg dialog




CSampleDlg::CSampleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSampleDlg::IDD, pParent)

{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSampleDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CSampleDlg::OnBnClickedButtonTest)
	ON_BN_CLICKED(IDC_BUTTON_CONN, &CSampleDlg::OnBnClickedButtonConn)
END_MESSAGE_MAP()


// CSampleDlg message handlers

BOOL CSampleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

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
	GetDlgItem(IDC_BUTTON_TEST)->EnableWindow(false);


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSampleDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// -----------------------------------------------------------------------------------

/*
use your_db;
-- +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
CREATE TABLE IF NOT EXISTS 
-- ----------------------------------------------------------
TEST_ME
-- ----------------------------------------------------------
(
-- NOT NULL SESSION
identity				  CHAR(36) 
NOT NULL,

test		            VARCHAR(256)  
NOT NULL,


CONSTRAINT PK_ PRIMARY KEY(identity)

) 
-- ----------------------------------------------------------
-- ----------------------------------------------------------
CHARACTER SET utf8 COLLATE utf8_unicode_ci
-- ----------------------------------------------------------
ENGINE=InnoDB; 
-- ----------------------------------------------------------

INSERT INTO TEST_ME VALUES(UUID(), 'Test 1');
INSERT INTO TEST_ME VALUES(UUID(), 'Test 2');
INSERT INTO TEST_ME VALUES(UUID(), 'Test 3');

SELECT * FROM TEST_ME;
*/

void CSampleDlg::OnBnClickedButtonConn()
{
	// TODO: Add your control notification handler code here

	try
	{	
		//theApp._pDataBase = new DataBase<MySqlDB>("127.0.0.1", "root", "root", "HIT");
	    theApp._pmysqldb = new MySqlDB("127.0.0.1", "root", "root", "HIT");

		GetDlgItem(IDC_BUTTON_CONN)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON_CONN)->SetWindowText("connected");
		GetDlgItem(IDC_BUTTON_TEST)->EnableWindow(true);
	}
	catch (const DataBaseError& e)
	{
		MessageBox(e.what());
	}

}

void CSampleDlg::OnBnClickedButtonTest()
{
	// TODO: Add your control notification handler code here

	try
	{	
		ResultSet rs;
		std::vector<std::string> row;
		std::string sdata;

		//(*theApp._pDataBase)  << "SELECT * FROM H_istudy", rs;
		theApp._pmysqldb->query("SELECT * FROM H_istudy");
		theApp._pmysqldb->populate(rs);

		while(rs.fetch(row))
		{
			for (size_t i = 0; i < row.size(); i++)
			{
				sdata += row[i] + " | ";
			}
		}

		MessageBox(sdata.c_str());
	}
	catch (const DataBaseError& e)
	{
		MessageBox(e.what());
	}

}