
// TestMariaDbAppDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "TestMariaDbApp.h"
#include "TestMariaDbAppDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CTestMariaDbAppDlg �Ի���




CTestMariaDbAppDlg::CTestMariaDbAppDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestMariaDbAppDlg::IDD, pParent)
	, m_strlog(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestMariaDbAppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_LOG, m_strlog);
}

BEGIN_MESSAGE_MAP(CTestMariaDbAppDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON1, &CTestMariaDbAppDlg::OnBnConnect)
	ON_BN_CLICKED(IDC_BUTTON2, &CTestMariaDbAppDlg::OnBnQuery)
	ON_BN_CLICKED(IDC_BUTTON3, &CTestMariaDbAppDlg::OnBnExcueSql)
END_MESSAGE_MAP()


// CTestMariaDbAppDlg ��Ϣ�������
BOOL StringToWString(const std::string &str,std::wstring &wstr)
{    
	int nLen = (int)str.length();    
	wstr.resize(nLen,L' ');

	int nResult = MultiByteToWideChar(CP_ACP,0,(LPCSTR)str.c_str(),nLen,(LPWSTR)wstr.c_str(),nLen);

	if (nResult == 0)
	{
		return FALSE;
	}

	return TRUE;
}
//wstring���ֽڲ�Ϊ0������FALSE
BOOL WStringToString(const std::wstring &wstr,std::string &str)
{    
	int nLen = (int)wstr.length();    
	str.resize(nLen,' ');

	int nResult = WideCharToMultiByte(CP_ACP,0,(LPCWSTR)wstr.c_str(),nLen,(LPSTR)str.c_str(),nLen,NULL,NULL);

	if (nResult == 0)
	{
		return FALSE;
	}

	return TRUE;
}

std::string W2S(const std::wstring &wstr)
{
	std::string str;
	WStringToString(wstr,str);
	return str;
}

std::wstring S2W(const std::string &str)
{
	std::wstring wstr;
	StringToWString(str,wstr);
	return wstr;
}

BOOL CTestMariaDbAppDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	GetDlgItem(IDC_IP)->SetWindowText(_T("127.0.0.1"));
	GetDlgItem(IDC_NAME)->SetWindowText(_T("root"));
	GetDlgItem(IDC_PWD)->SetWindowText(_T("root"));
	GetDlgItem(IDC_DATANAME)->SetWindowText(_T("HIT"));

	GetDlgItem(IDC_SQL)->SetWindowText(_T("SELECT * FROM H_istudy"));
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CTestMariaDbAppDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CTestMariaDbAppDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CTestMariaDbAppDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CTestMariaDbAppDlg::OnBnConnect()
{
	try
	{	
		CString strIP("127.0.0.1"),strUser("root"),strPwd("root"),strDadaName("HIT");
		GetDlgItem(IDC_IP)->GetWindowText(strIP);
		GetDlgItem(IDC_NAME)->GetWindowText(strUser);
		GetDlgItem(IDC_PWD)->GetWindowText(strPwd);
		GetDlgItem(IDC_DATANAME)->GetWindowText(strDadaName);

		pMariaDb = new HMariaDb(W2S(strIP.GetBuffer()).c_str(), W2S(strUser.GetBuffer()).c_str(), \
			                    W2S(strPwd.GetBuffer()).c_str(),W2S(strDadaName.GetBuffer()).c_str());///*"127.0.0.1"*/"root", "root", "HIT");

		GetDlgItem(IDC_BUTTON1)->EnableWindow(false);
		CString datainfo = CString("IP:") + strIP;
		datainfo += CString("| user:");
		datainfo += strUser;
		datainfo += CString("| pwd:");
		datainfo += CString("| database name:");
		datainfo += strDadaName;
		datainfo += CString(" @ connected ok!\r\n");
		GetDlgItem(IDC_BUTTON1)->SetWindowText(_T("connected"));
		GetDlgItem(IDC_BUTTON2)->EnableWindow(true);

		m_strlog += datainfo;
		GetDlgItem(IDC_LOG)->SetWindowText(m_strlog);
/*		MessageBox(datainfo);*/

		srand((unsigned)time(NULL));
	}
	catch (const HSqlError& e)
	{
		MessageBox(S2W(e.what()).c_str());
	}
}

void CTestMariaDbAppDlg::OnBnQuery()
{
	try
	{	
		std::vector<std::string> row;
		std::string strsql = "SELECT * FROM H_istudy";
		CString wsql;
		GetDlgItem(IDC_SQL)->GetWindowText(wsql);
		if (wsql.GetLength()>1)
		{
			strsql = W2S(wsql.GetBuffer());
		}
		pMariaDb->query(strsql);
/*		pMariaDb->getresult(rs);*/
		ResultSet * rs = pMariaDb->QueryResult();
		if (NULL != rs)
		{
			std::string sdata;
			while(rs->fetch(row))
			{
				for (size_t i = 0; i < row.size(); i++)
				{
					sdata += row[i] + " | ";
				}
				sdata += "\r\n";
			}
			std::wstring strinfo;
			StringToWString(sdata,strinfo);
			//MessageBox(strinfo.c_str());
			m_strlog += strinfo.c_str();
			GetDlgItem(IDC_LOG)->SetWindowText(m_strlog);
		}
		int a = rand();
		strsql = "insert into H_istudy (patientid,studyuid) value(";

		CString s;
		s.Format(_T("%d"),a);
		std::string v = W2S(s.GetBuffer());
		strsql += v;
		strsql += ",'1.2.826.0.1.3680043.9.7604.";
		strsql += v;
		strsql += "');";
		pMariaDb->execute(strsql);
	}
	catch (const HSqlError& e)
	{
		MessageBox(S2W(e.what()).c_str());
	}
}

void CTestMariaDbAppDlg::OnBnExcueSql()
{
	if (pMariaDb == NULL)
		return;
	try
	{	
		ResultSet rs;
		std::vector<std::string> row;
		std::string sdata;
		std::string strsql;

		int a = rand()+1000000;
		strsql = "update H_istudy set studyuid = ";

		CString s;
		s.Format(_T("%d"),a);
		std::string v = W2S(s.GetBuffer());
		strsql += "'1.2.826.0.1.3680043.9.7604.";
		strsql += v;
		strsql += "' where patientid = 7666;";
		pMariaDb->execute(strsql);
	}
	catch (const HSqlError& e)
	{
		MessageBox(S2W(e.what()).c_str());
	}
}
