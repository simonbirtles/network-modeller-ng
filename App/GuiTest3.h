#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

namespace NM
{
	namespace Registry
	{
		class IServiceRegistry;
	}
}

class CGuiTest3App : public CBCGPWinApp
{
public:
	CGuiTest3App();
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();


	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;
	
	//afx_msg void OnAppAbout();
	afx_msg void OnFileSave();
	DECLARE_MESSAGE_MAP()

private:
	void RegisterCoreServices();
	void LoadDatabase();
	void NewNode();
	void NewLink();
	void NewFlow();
	void NewDemand();
};



extern CGuiTest3App theApp;
