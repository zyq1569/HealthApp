
// TestMariaDbAppDlg.h : ͷ�ļ�
//

#pragma once
#include "HMariaDb.h"

// CTestMariaDbAppDlg �Ի���
class CTestMariaDbAppDlg : public CDialog
{
// ����
public:
	CTestMariaDbAppDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_TESTMARIADBAPP_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	HMariaDb *pMariaDb;
	afx_msg void OnBnConnect();
	afx_msg void OnBnQuery();
	// ��Ϣ��ʾ
	CString m_strlog;
};
