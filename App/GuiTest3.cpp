#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "GuiTest3.h"
#include "MainFrm.h"

//#include "ChildFrm.h"
//#include "GuiTest3Doc.h"
//#include "GuiTest3View.h"

#include "NetworkViewFrame.h"
#include "NetworkViewDoc.h"
#include "NetworkView.h"

#include "TrafficEdit\TrafficEditFrame.h"
#include "TrafficEdit\TrafficEditDoc.h"
#include "TrafficEdit\TrafficEditPathView.h"


// registry service
#include "ServiceRegistryCommon.h"
#include "IServiceRegistry.h"						// registry interface
//#include "IServiceRegistry.cpp"						// registry interface (reqd due to tmeplates)

// reqd for both serialization interface and for service registry class registration.
#include "ISerial.h"										// for client interface 
#include "ISerialService.h"								// for service registration

// reqd for database update service
#include "Interfaces\IDatabaseUpdateService.h"			// Create a service for Update 

// reqd for client database update notifications
#include "Interfaces\IDatabaseUpdate.h"					// for client interface 
#include "Interfaces\DatabaseObserver.h"					// base class for client listener (dervive from for update calls)

// reqd for objectdatabase 
#include "Interfaces\IObjectDatabaseService.h"			// for service registration
#include "Interfaces\IObjectDatabase.h"					// for client interface 
#include "Interfaces\Key.h"	

// reqd for services (selected objected)
#include "ISelectedObjectsService.h"

//#include "IIPService.h"							// client interface (need this here ???)
#include "IInternetProtocolService.h"			// service interface for IP Services

#include "AdjacencyMatrixService.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/**
* Global Inits/Decls
*
*
*/
CGuiTest3App theApp;							// The one and only CGuiTest3App object
NM::Registry::IServiceRegistry* reg;
/**
*
*
*
*/
BEGIN_MESSAGE_MAP(CGuiTest3App, CBCGPWinApp)
	ON_COMMAND(ID_FILE_SAVE_DB, &CGuiTest3App::OnFileSave)
	ON_COMMAND(ID_NEWNODE, &CGuiTest3App::NewNode)
	ON_COMMAND(ID_NEWLINK, &CGuiTest3App::NewLink)
	ON_COMMAND(ID_NEWFLOW, &CGuiTest3App::NewFlow)
	ON_COMMAND(ID_NEWDEMAND, &CGuiTest3App::NewDemand)

END_MESSAGE_MAP()
/**
*
*
*
*/
CGuiTest3App::CGuiTest3App()
{
	m_bHiColorIcons = TRUE;
	// format for string is CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(L"CNS.NetworkModel.Version2.0");
}
/**
*
*
*
*/
BOOL CGuiTest3App::InitInstance()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(252892);

	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CBCGPWinApp::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();
	EnableTaskbarInteraction();
	AfxInitRichEdit2();			// is required to use RichEdit control	
	SetRegistryKey(L"CNS");
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)
	//EnableD2DSupport();
	InitContextMenuManager();
	InitKeyboardManager();
	InitTooltipManager();

	//CMFCToolTipInfo ttParams;
	CBCGPToolTipParams ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	// Restore the standard tooltip:
	theApp.GetTooltipManager()->SetTooltipParams(BCGP_TOOLTIP_TYPE_ALL, NULL, NULL);
	//theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL, RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);



	// Register the application's document templates.  Document templates serve as the connection between documents, frame windows and views
	CMultiDocTemplate* pDocNetworkView;
	pDocNetworkView = new CMultiDocTemplate(IDR_GuiTest3TYPE,				// need to create new IDR and associated stgrings
		RUNTIME_CLASS(CNetworkViewDoc),
		RUNTIME_CLASS(CNetworkViewFrame), // custom MDI child frame
		RUNTIME_CLASS(CNetworkView));
	if (!pDocNetworkView)	return FALSE;
	AddDocTemplate(pDocNetworkView);


	//CMultiDocTemplate* pDocTrafficEdit;
	//pDocTrafficEdit = new CMultiDocTemplate(IDR_GuiTest3TYPE,				// need to create new IDR and associated stgrings
	//	RUNTIME_CLASS(CTrafficEditDoc),
	//	RUNTIME_CLASS(CTrafficEditFrame), // custom MDI child frame
	//	RUNTIME_CLASS(CTrafficEditPathView));
	//if (!pDocTrafficEdit)	return FALSE;
	//AddDocTemplate(pDocTrafficEdit);


	// Load & start all core network modelling services
	RegisterCoreServices();
	// Load database - static filename currently
	LoadDatabase();

	// create main MDI Frame window, and therefore creates
	// all child windows such as outputwnd, and docking windows. 
	// need to have services loaded and db loaded before as they 
	// will call for the data, code does not support loading
	// data after mainframe loaded yet... 
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME)){
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;
	

	// Open the graph network document
	CDocument* pNetworkDoc = pDocNetworkView->OpenDocumentFile(NULL);
	pNetworkDoc->SetTitle(L"Network");	

	//CDocument* pTrafficDoc = pDocTrafficEdit->OpenDocumentFile(NULL);
	//pTrafficDoc->SetTitle(L"Traffic Demands");

	// The main window has been initialized, so show and update it
	pMainFrame->ShowWindow(SW_SHOWMAXIMIZED);
	pMainFrame->UpdateWindow();	
	return TRUE;
}
/**
*
*
*
*/
void WCtMB(const ::std::wstring &ws, ::std::string &mbs)
{
	const int max_string_length = 255;
	char chartemp[max_string_length];
	memset(chartemp, 0x0, max_string_length);
	size_t i;

	setlocale(LC_ALL, ".ACP");
	int ret = wcstombs_s(&i, chartemp, max_string_length, ws.c_str(), max_string_length - 1);
	if (ret == max_string_length)
	{
		chartemp[max_string_length - 1] = NULL;
	}

	mbs.clear();
	mbs.append(chartemp);

	return;
}
/**
*
*
*
*/
void MBtWC(::std::wstring &ws, const ::std::string &mbs)
{
	const int max_string_length = 255;
	wchar_t *wc = new wchar_t[max_string_length];
	memset(wc, 0x0, sizeof(wchar_t)*max_string_length);

	size_t converted = 0;

	setlocale(LC_ALL, ".ACP");

	mbstowcs_s(&converted, wc, max_string_length - 1, mbs.c_str(), mbs.size());

	ws = wc;

	delete[] wc;
	wc = nullptr;

	return;
}
/**
*
*
*
*/
int CGuiTest3App::ExitInstance()
{
	BCGCBProCleanUp();
	AfxOleTerm(FALSE);
	delete reg;
	reg = nullptr;
	return CBCGPWinApp::ExitInstance();
}
/**
* RegisterCoreServices
*
* In order, load the core services into the registry and start them
*/
void CGuiTest3App::RegisterCoreServices()
{
	// start registry, global var
	reg = new NM::Registry::IServiceRegistry;

	::std::vector<::std::wstring>  serviceDependencies;


	// **************************************************************
	// Register & start system message bus
	serviceDependencies.clear();
	// todo

	//Register & start Serialization service
	serviceDependencies.clear();
	reg->RegisterService<::NM::Serial::ISerialService>(L"File", L"File Serialization Functions", serviceDependencies);
	reg->StartRegService(L"File");
	
	//Register & start DatabaseUpdateCache service
	serviceDependencies.clear();
	reg->RegisterService<::NM::ODB::IDatabaseUpdateService>(L"ObjectUpdateCache", L"Database Client Updates", serviceDependencies);
	reg->StartRegService(L"ObjectUpdateCache");

	// **************************************************************
	// Register & start data helper services (i.e. Adj Matrix etc )
	serviceDependencies.clear();
	serviceDependencies.push_back(L"ObjectUpdateCache");
	serviceDependencies.push_back(L"ODB");
	reg->RegisterService<::NM::DataServices::AdjacencyMatrixService>(L"ADJMATRIX", L"Adjacency Matrix Service", serviceDependencies);
	reg->StartRegService(L"ADJMATRIX");

	//Register & start Object database service
	serviceDependencies.clear();
	serviceDependencies.push_back(L"File");
	serviceDependencies.push_back(L"ObjectUpdateCache");
	reg->RegisterService<::NM::ODB::IObjectDatabaseService>(L"ODB", L"Core Object Database", serviceDependencies);
	reg->StartRegService(L"ODB");

	//Register & start selected objects service
	serviceDependencies.clear();
	reg->RegisterService<::NM::ISelectedObjectsService>(L"SELECTEDOBJECTS", L"Selected Objects Info", serviceDependencies);
	reg->StartRegService(L"SELECTEDOBJECTS");

	//Register & start IP Service
	serviceDependencies.clear();
	serviceDependencies.push_back(L"ODB");
	serviceDependencies.push_back(L"ObjectUpdateCache");
	reg->RegisterService<::NM::IPRTREE::IInternetProtocolService>(L"IPSERVICE", L"Provides Internet Protocol Services", serviceDependencies);
	reg->StartRegService(L"IPSERVICE");

	return;
}
/**
* LoadDatabase()
*
*
*/
void CGuiTest3App::LoadDatabase()
{
	// Load Database File (default)
	::std::unique_ptr<NM::Serial::ISerial> serial;
	serial.reset(reinterpret_cast<NM::Serial::ISerial*>(reg->GetClientInterface(L"File")));
	//::std::wstring loadFilename = L"C:\\Users\\Simon\\Desktop\\networkmodeller2.xml";
	::std::wstring loadFilename = L"C:\\Users\\Simon\\Desktop\\networkmodeller2-intf.xml";
	bool fileLoaded = serial->LoadDatabase(loadFilename);
	if (!fileLoaded)
	{
		OutputDebugString(L"\nDB NOT File Loaded..., fall out to do... ");
		assert(false);		
	}
	//delete serial;
	//serial = nullptr;
	return;
}
/**
*
*
*
*/
void CGuiTest3App::OnFileSave()
{
	DWORD ret = ::MessageBox(NULL, L"Save ?", L"Save?", MB_YESNO);
	if (ret == IDNO) return;

	NM::Serial::ISerial* serial = reinterpret_cast<NM::Serial::ISerial*>(reg->GetClientInterface(L"File"));
	//::std::wstring saveFilename = L"C:\\Users\\Simon\\Desktop\\networkmodeller2.xml";
	::std::wstring saveFilename = L"C:\\Users\\Simon\\Desktop\\networkmodeller2-intf.xml";
	bool fileSaved = serial->SaveDatabase(saveFilename);
	if (!fileSaved)
	{
		assert(false);
		OutputDebugString(L"\nDB NOT SAVED !");
	}
	delete serial;
	serial = nullptr;
	
}
/**
*
*
*
*/
void CGuiTest3App::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	bNameValid = strName.LoadString(IDS_EXPLORER);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);
	GetContextMenuManager()->AddMenu(L"ipprottreeview", IDR_POPUP_IPTREEVIEW);
}
/**
*
*
*
*/
void CGuiTest3App::LoadCustomState()
{
}
/**
*
*
*
*/
void CGuiTest3App::SaveCustomState()
{
}
/**
*
*
*
*/
void CGuiTest3App::NewNode()
{
	::MessageBox(NULL, L"NewNode", L"NewNode", MB_OK);
}
/**
*
*
*
*/
void CGuiTest3App::NewLink()
{
	::MessageBox(NULL, L"NewLink", L"NewLink", MB_OK);
}/**
*
*
*
*/
void CGuiTest3App::NewFlow()
{
	::MessageBox(NULL, L"NewFlow", L"NewFlow", MB_OK);
}
/**
*
*
*
*/
void CGuiTest3App::NewDemand()
{
	::MessageBox(NULL, L"NewDemand", L"NewDemand", MB_OK);
}






