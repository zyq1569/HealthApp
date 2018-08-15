
// Sample.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

//
// MySql header here!
#include "MysqlDb.h"
//
//


// CSampleApp:
// See Sample.cpp for the implementation of this class
//

class CSampleApp : public CWinAppEx
{
public:
	CSampleApp();

	~CSampleApp()
	{
		//if(_pDataBase != NULL)
		//{
		//	delete _pDataBase;
		//}
	}

// Overrides
	public:
	virtual BOOL InitInstance();

	//DataBase<MySqlDB> *_pDataBase;
	MySqlDB *_pmysqldb;


// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CSampleApp theApp;