// PRT.h : main header file for the PRT application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CPRTApp:
// See PRT.cpp for the implementation of this class
//

class CPRTApp : public CWinApp
{
public:
	CPRTApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CPRTApp theApp;