/**
* NetworkView.cpp (MFC)
*
* Addtional cpp file for this class is NetworkViewWndProc, contains; 
*		overridden WndProc for this windo
*		DirectX Creation/Resource management.
*
*/
#include "stdafx.h"
#include "GuiTest3.h"
#include "NetworkView.h"
#include "IServiceRegistry.h"
#include "INetworkViewService.h"
#include "Interfaces\IObjectDatabase.h"					// for client interface to core database 
#include "GroupViewMetadata.h"
#include "NetworkViewSpatialHash.h"
#include "BitmapCache.h"


//#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "d2d1.lib")
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "WindowsCodecs.lib")
#pragma comment (lib, "dwrite.lib")

#pragma region MFCSTUFF
IMPLEMENT_DYNCREATE(CNetworkView, CView)
BEGIN_MESSAGE_MAP(CNetworkView, CView)
	//ON_REGISTERED_MESSAGE(AFX_WM_UPDATETOOLTIPS, &CNetworkView::OnUpdateToolTips)
END_MESSAGE_MAP()

// CNetworkView diagnostics
#ifdef _DEBUG
void CNetworkView::AssertValid() const
{
	CView::AssertValid();
}

#ifndef _WIN32_WCE
void CNetworkView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif
#endif //_DEBUG
#pragma endregion MFCSTUFF

extern ::NM::Registry::IServiceRegistry* reg;

/**
*
*
*
*/
CNetworkView::CNetworkView() :
	_DPIScaleX(0),
	_DPIScaleY(0),
	_bRedraw(true),
	bHaveFocus(false),
	_bClientRenderUpdateRequired(true),
	_pLastLayerMessageProcessed(nullptr),
	_breadcrumbHeight(25),
	_scrollX(0),
	_scrollY(0),
	_scaleX(1.0f),
	_scaleY(1.0f),
	_bRightButtonDown(false),
	_bLeftButtonDown(false),
	_bTrackingMouse(false),
	_bDisplayingToolTip(false),
	_hwndTrackingToolTip(NULL),
	_bEnableToolTips(false),
	_trackPT({ -1,-1 }),
	_lastMousePosition({-1,-1}),
	_currentLayerUID(::NM::ODB::INVALID_OBJECT_UID)

{
	/*
	#define AFX_TOOLTIP_TYPE_DEFAULT    0x0001
	#define AFX_TOOLTIP_TYPE_TOOLBAR    0x0002
	#define AFX_TOOLTIP_TYPE_TAB        0x0004
	#define AFX_TOOLTIP_TYPE_MINIFRAME  0x0008
	#define AFX_TOOLTIP_TYPE_DOCKBAR    0x0010
	#define AFX_TOOLTIP_TYPE_EDIT       0x0020
	#define	AFX_TOOLTIP_TYPE_BUTTON     0x0040
	#define	AFX_TOOLTIP_TYPE_TOOLBOX    0x0080
	#define AFX_TOOLTIP_TYPE_PLANNER    0x0100
	#define AFX_TOOLTIP_TYPE_RIBBON     0x0200
	#define	AFX_TOOLTIP_TYPE_CAPTIONBAR 0x0400
	*/
	tooltiptype = AFX_TOOLTIP_TYPE_BUTTON; //AFX_TOOLTIP_TYPE_TOOLBAR;
	_spatialHash.reset(new ::NM::NetGraph::NetworkViewSpatialHash(32));
	_bitmapCache.reset( new ::NM::NetGraph::BitmapCache );
}
/**
*
*
*
*/
CNetworkView::~CNetworkView()
{
	DiscardDeviceResources();
	LAYER_TO_TARGET_MAP::iterator it = m_pLayerTargetMap.begin();
	while (it != m_pLayerTargetMap.end())
	{
		delete it->first;
		++it;
	}
	m_pLayerTargetMap.clear();
}
/**
*
*
*
*/
bool CNetworkView::CreateRegistryService()
{
	::NM::NetGraph::INetworkViewService::_pNetworkView = this;

	// if the global registry is not available we are in trouble
	if (!reg)
		throw ::std::runtime_error("Application Registry Not Available, Cannot Continue.");

	::std::vector<::std::wstring> dep;
	bool registered = reg->RegisterService<::NM::NetGraph::INetworkViewService>(L"NETWORKVIEW", L"Network GraphView Client Interface", dep);
	if (!registered)
		throw ::std::runtime_error("Failed to register INetworkViewService");

	bool serviceStarted = reg->StartRegService(L"NETWORKVIEW");
	if (!serviceStarted)
		throw ::std::runtime_error("Failed to start INetworkViewService");

	// Get The Database Handle and save
	_odb.reset(reinterpret_cast<NM::ODB::IObjectDatabase*>(reg->GetClientInterface(L"ODB")));
	if (!_odb)
		throw ::std::runtime_error("Database Not Available, Cannot Continue.");

	return serviceStarted;
}
/**
* CreateClient
*
* really temp function until a method is built to allow dynamic registration 
*/
bool CNetworkView::CreateClient(::NM::NetGraph::BaseLayer* pLayer)
{	
	if (pLayer != nullptr)
	{
		// save 
		CreateClientCache(pLayer, -1);
		// call Initialise
		pLayer->OnInitialise(this);
	}

	return true;
}
/**
* UpdateClientCache
*
* This inserts a new client layer or reorders a client based on a new zpos and stores in local cache of client layers
*/
bool CNetworkView::CreateClientCache(::NM::NetGraph::BaseLayer* pBaseLayer, int layerRenderPosition)
{
	// create a render target for this layer
	::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pBitmapRenderTarget;
	HRESULT hr = m_pDeviceContext->CreateCompatibleRenderTarget(&pBitmapRenderTarget);
	
	LAYER_TO_TARGET_MAP::iterator targetit = m_pLayerTargetMap.find(pBaseLayer);
	if (targetit != m_pLayerTargetMap.end())
	{
		targetit->second.Reset();
	}
	m_pLayerTargetMap[pBaseLayer] = pBitmapRenderTarget;
	
	// if the layer already exists in the render stack, then adjust index position if different
	LAYER_RENDER_ORDER::iterator it = ::std::find(_layerRenderOrder.begin(), _layerRenderOrder.end(), pBaseLayer);
	if (it != _layerRenderOrder.end())
	{
		size_t currentIndex = ::std::distance(_layerRenderOrder.begin(), it);
		if (layerRenderPosition != currentIndex)
		{
			// erase it and later code will re-add in the requested pos if valid
			_layerRenderOrder.erase(it);
		}
	}
	
	// doesnt already exist (or has just been removed to change index pos) and requested position is invalid
	if ((layerRenderPosition < 0) || (layerRenderPosition > static_cast<int>(_layerRenderOrder.size()) - 1))
	{
		_layerRenderOrder.push_back(pBaseLayer);
	}
	// does not exist (or has just been removed to change index pos), valid position, so insert into vector at pos
	else
	{
		_layerRenderOrder.insert(_layerRenderOrder.begin() + layerRenderPosition, pBaseLayer);
	}

	return true;
}
/**
* GetRenderTarget
*
* Called from client layers to get the current rendertarget, usually in response to a change the layer 
* needs to make and not due to a OnRender call.
*/
HRESULT CNetworkView::GetRenderTarget(::NM::NetGraph::BaseLayer* pLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> &pRenderTarget)
{
	HRESULT hr = S_OK;
	LAYER_TO_TARGET_MAP::iterator it = m_pLayerTargetMap.find(pLayer);
	if (it != m_pLayerTargetMap.end())
	{
		assert(it->second != nullptr);
		pRenderTarget = it->second;
		return S_OK;
	}
	return E_FAIL;
}
/**
* OnRender
*
* Clears the window and calls each client layer from bottom up z-order to 
* render thier content to the rendertarget, then we present the final rendertarget
* for drawing to the screen.
*/
HRESULT CNetworkView::OnRender()
{
	HRESULT hr = S_OK;

	// if this was set to true, all clients have now had
	// a OnRender call so all RenderTargets will be up to date 
	// and no longer need to request a redraw from layers, 
	// when they get called with bRedraw=false, we are just
	// informing them we are drawing and will be using current
	// bitmap and to update/redraw only if they need to. 
	_bRedraw = false;

	// check we have a DeviceContext to draw on.
	if (!m_pDeviceContext)
	{
		hr = CreateDiscardableResources();
		ASSERT(!FAILED(hr));
	}
	
	RECT viewMargins;
	GetViewMargins(viewMargins);
	D2D1_SIZE_F sizeF = m_pDeviceContext->GetSize();
	D2D1_RECT_F destPos = { 0 };
	destPos.top = viewMargins.top + 1;
	destPos.right = sizeF.width;
	destPos.bottom = sizeF.height;

	// move to dedicated class owned by NetworkView  (BreadCrumb)
	::MoveWindow(_breadcrumb->GetSafeHwnd(), 0, 0, sizeF.width, _breadcrumbHeight, TRUE);

	m_pDeviceContext->BeginDraw();
	m_pDeviceContext->Clear(D2D1::ColorF(0.2f, 0.2f, 0.2f, 1.0f));

	/*
	* Get layer bitmaps and draw them in order
	*/
	LAYER_RENDER_ORDER::iterator renderit = _layerRenderOrder.begin();
	while (renderit != _layerRenderOrder.end())
	{
		LAYER_TO_TARGET_MAP::iterator targetit = m_pLayerTargetMap.find(*renderit);
		if (targetit != m_pLayerTargetMap.end())
		{
			if ((targetit->second != nullptr) && targetit->first->ShouldRender())
			{
				::Microsoft::WRL::ComPtr<ID2D1Bitmap> pBmpLayer;
				hr = targetit->second->GetBitmap(&pBmpLayer);
				if (SUCCEEDED(hr))
				{
					D2D1_SIZE_F bmpSize = pBmpLayer->GetSize();
					D2D1_RECT_F srcBmpSize = { 0 };
					srcBmpSize.right = bmpSize.width;
					srcBmpSize.bottom = bmpSize.height;
					m_pDeviceContext->DrawBitmap(pBmpLayer.Get(), &destPos, 1.0f, D2D1_INTERPOLATION_MODE_LINEAR, &srcBmpSize); //D2D1_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC
				}
				else
				{
					throw ::std::runtime_error("Failed to get layer bmp!");
				}
				pBmpLayer.Reset();
			}
		}
		else
		{
			throw ::std::runtime_error("Layer bmp NULL !!!!!!");
		}
		++renderit;
	}

	hr = m_pDeviceContext->EndDraw();

	// and ... Present (new for Direct2D 1.1)
	DXGI_PRESENT_PARAMETERS parameters = { 0 };
	parameters.DirtyRectsCount = 0;
	parameters.pDirtyRects = nullptr;
	parameters.pScrollRect = nullptr;
	parameters.pScrollOffset = nullptr;
	hr = m_pSwapChain->Present1(1, 0, &parameters);

	if (hr == D2DERR_RECREATE_TARGET)
	{
		hr = DiscardDeviceResources();
	}
	return hr;
}
/**
*
*
*
*/
void CNetworkView::GetViewMargins(RECT& margins)
{
	margins = { 0 };
	margins.top = _breadcrumbHeight;
}
/**
*
*
*
*/
void CNetworkView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	SCROLLINFO si;
	::std::memset(&si, 0, sizeof(SCROLLINFO));

	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_RANGE | SIF_POS;
	si.nMin = 0;
	si.nMax = 1000;
	si.nPos = 0;

	SetScrollInfo(SB_HORZ, &si);
	SetScrollInfo(SB_VERT, &si);

	// move to dedicated class owned by NetworkView (BreadCrumb)
#pragma region bctest 
	
	RECT rectBreadcrumb;
	rectBreadcrumb.top = 0;
	rectBreadcrumb.left = 0;
	rectBreadcrumb.bottom = _breadcrumbHeight;
	rectBreadcrumb.right = 500;

	_breadcrumb.reset(new BreadCrumb(this));
	_breadcrumb->Create(rectBreadcrumb, this, IDC_BREADCRUMB, WS_CHILD | WS_VISIBLE, BCCS_SHOWROOTALWAYS);
	_breadcrumb->m_bVisualManagerStyle = TRUE;
	
	HBREADCRUMBITEM hRoot = _breadcrumb->GetRootItem();
	_breadcrumb->SetItemText(hRoot, _T("WORLD"));

	HBREADCRUMBITEM hMusic = _breadcrumb->InsertItem(hRoot, _T("LONDON"), -1);
	HBREADCRUMBITEM hPic = _breadcrumb->InsertItem(hMusic, _T("TELEHOUSE"), -1);

	_breadcrumb->SelectPath(CString("WORLD\\LONDON\\TELEHOUSE"));

#pragma endregion bctest
	return;
}
/**
*
*
*
*/
void CNetworkView::TransformPoints(POINT &p)
{
	//  (BreadCrumb)
	/*if(p.y > _breadcrumbHeight)*/
	RECT viewMargins;
	GetViewMargins(viewMargins);
	p.y -= viewMargins.top + 1;

	// transform (TRANSLATIONSET)
	D2D1::Matrix3x2F translation = D2D1::Matrix3x2F::Translation(
		::D2D1::SizeF(
			static_cast<float>(_scrollX),
			static_cast<float>(_scrollY))
		);

	D2D1::Matrix3x2F scale = D2D1::Matrix3x2F::Scale(
		::D2D1::SizeF(
			_scaleX,
			_scaleY),
		::D2D1::Point2F(0.0f, 0.0f
			));

	D2D1::Matrix3x2F m = translation * scale;

	m.Invert();

	D2D1_POINT_2F transformedPoints = m.TransformPoint(::D2D1::Point2F(static_cast<float>(p.x), static_cast<float>(p.y)));
	p.x = static_cast<long>(transformedPoints.x);
	p.y = static_cast<long>(transformedPoints.y);
	return;
}
/**
*
*
*
*/
void CNetworkView::UpdateRenderTargetTransforms()
{
	//OutputDebugString(L"\nRender Info: \n\tTransforms:");
	//OutputDebugString(L"\n\t\t Translation:\t");
	//OutputDebugString(::std::to_wstring((float)_scrollX).c_str());		// or _32
	//OutputDebugString(L"\t");
	//OutputDebugString(::std::to_wstring((float)_scrollY).c_str());		// or _32
	//OutputDebugString(L"\n\t\t Scale:\t\t\t");
	//OutputDebugString(::std::to_wstring(_scaleX).c_str());		// or _22
	//OutputDebugString(L"\t");
	//OutputDebugString(::std::to_wstring(_scaleY).c_str());		// or _22

	LAYER_TO_TARGET_MAP::iterator it = m_pLayerTargetMap.begin();
	while (it != m_pLayerTargetMap.end())
	{
		if (it->second != nullptr)
		{
			D2D1::Matrix3x2F translation = D2D1::Matrix3x2F::Translation(
				::D2D1::SizeF(
					static_cast<float>(_scrollX), 
					static_cast<float>(_scrollY))
				);

			D2D1::Matrix3x2F scale = D2D1::Matrix3x2F::Scale(
				::D2D1::SizeF(
					_scaleX, 
					_scaleY), 
				::D2D1::Point2F(0.0f, 0.0f
					));
			
			it->second->SetTransform(translation * scale);						// transform
			if(it->first->ShouldRender())
				it->first->OnRender(it->second, true);
		}
		++it;
	}
	return;
}
/**
* SetTranslation
*
* Set translation: x,y are relative values, use 0 for no change of a plane translation
* absolute flag not impl. yet
*/
void CNetworkView::SetTranslation(int x, int y, bool absolute)
{
	if (absolute)
	{
		_scrollX = (min(0, x));
		_scrollY = (min(0, y));
	}
	else
	{
		_scrollX = (min(0, _scrollX + x));
		_scrollY = (min(0, _scrollY + y));
	}

	::SetScrollPos(this->GetSafeHwnd(), SB_HORZ, -_scrollX, TRUE);
	::SetScrollPos(this->GetSafeHwnd(), SB_VERT, -_scrollY, TRUE);


	return;
}
/**
* SetScale
*
* Set scale: x,y are relative values to current value (aggregate), use 0 for no change of scale of a plane
* absolute flag not impl. yet
*/
void CNetworkView::SetScale(float scale, bool absolute)
{
	if (absolute)
	{
		_scaleY = _scaleX = (scale < 0 ? max(0.4f, scale) : min(1.6f, scale));
	}
	else
	{
		_scaleY = _scaleX = (scale < 0 ? max(0.4f, _scaleX += scale) : min(1.6f, _scaleX += scale));
	}
	return;
}
/**
* SetGroupView
*
* INVALID_OBJECT_UID means root group.
*/
bool CNetworkView::SetGroupView(::NM::ODB::OBJECTUID groupUID)
{
	OutputDebugString(L"\n\tCNetworkView::SetGroupView");
	::NM::ODB::OBJECTUID layeruid = groupUID;
	::std::wstring layerString = _odb->GetObjectTypeName(layeruid);
	if( (layerString == L"group") || groupUID == ::NM::ODB::INVALID_OBJECT_UID)
	{
		// save the currently active group metadata
		SaveGroupMetaData();
		// change active group to requested group
		_currentLayerUID = layeruid;
		// load the newly active group metadata
		LoadGroupMetaData();
		// refresh client render targets and inform clients to redraw
		UpdateRenderTargetTransforms();
		// send update to all clients
		MessagePipeLine(NVLM_GROUPDISPLAYCHANGE, (WPARAM)&groupUID, 0, true);
		// redraw
		OnRender();

		return true;
	}
	return false;
}
/**
*
*
*
*/
::std::shared_ptr<::NM::NetGraph::NetworkViewSpatialHash> CNetworkView::GetSpatialHashPtr()
{
	return _spatialHash;
}
/**
*
*
*
*/
::std::shared_ptr<::NM::NetGraph::BitmapCache> CNetworkView::GetBitmapCachePtr()
{
	return _bitmapCache;
}
/**
*
*
*
*/
void CNetworkView::SaveGroupMetaData()
{
	GROUP_METADATA_MAP::iterator it = _groupMetaData.find(_currentLayerUID);
	if (it != _groupMetaData.end())
	{
		it->second->SetScale(_scaleX, _scaleY);
		it->second->SetTranslation(_scrollX, _scrollY);
	}
	else /* create new metadata class instance and save current metadata */
	{
		::std::shared_ptr<GroupViewMetadata> sp = ::std::make_shared<GroupViewMetadata>(_currentLayerUID);
		sp->SetScale(_scaleX, _scaleY);
		sp->SetTranslation(_scrollX, _scrollY);
		_groupMetaData[_currentLayerUID] = sp;
	}
	return;
}
/**
*
*
*
*/
void CNetworkView::LoadGroupMetaData()
{
	GROUP_METADATA_MAP::iterator it = _groupMetaData.find(_currentLayerUID);
	if (it != _groupMetaData.end())
	{
		it->second->GetScale(_scaleX, _scaleY);
		it->second->GetTranslation(_scrollX, _scrollY);

		SetScale(_scaleX, true);
		SetTranslation(_scrollX, _scrollY, true);

	}
	else /* create new metadata class instance with defaults */
	{
		::std::shared_ptr<GroupViewMetadata> sp = ::std::make_shared<GroupViewMetadata>(_currentLayerUID);
		_scaleX = _scaleY = 1.0f;
		_scrollX = _scrollY = 0;
		sp->SetScale(1.0f, 1.0f);
		sp->SetTranslation(0, 0);
		_groupMetaData[_currentLayerUID] = sp;
		SetScale(_scaleX, true);
		SetTranslation(_scrollX, _scrollY, true);
	}
	return;
}
/**
*
*
*
*/
void CNetworkView::CreateTrackingToolTip()
{
	////_hwndTrackingToolTip = CreateWindowEx(
	////	WS_EX_TOPMOST, 
	////	TOOLTIPS_CLASS, 
	////	NULL,
	////	WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,// | TTS_USEVISUALSTYLE,
	////	CW_USEDEFAULT, 
	////	CW_USEDEFAULT, 
	////	CW_USEDEFAULT, 
	////	CW_USEDEFAULT, 
	////	this->GetSafeHwnd(), 
	////	NULL, 
	////	AfxGetApp()->m_hInstance, 
	////	NULL);

	////assert(_hwndTrackingToolTip != NULL);
	////::std::memset(&_toolInfo, 0, sizeof(TOOLINFO));

	////_toolInfo.cbSize = sizeof(TOOLINFO);
	////_toolInfo.uFlags = TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE;
	////_toolInfo.hwnd = this->GetSafeHwnd();
	////_toolInfo.hinst = AfxGetApp()->m_hInstance;
	////_toolInfo.lpszText = L"1stline\r\n2ndline\r\n3rdline";
	////_toolInfo.uId = (UINT_PTR)this->GetSafeHwnd();;

	////::GetClientRect(this->GetSafeHwnd(), &_toolInfo.rect);

	////::SendMessage(_hwndTrackingToolTip, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&_toolInfo);
	////::SendMessage(_hwndTrackingToolTip, TTM_SETMAXTIPWIDTH, 0, 150);

	///////////////////////////////////////////////////////////////////////////////////////



	// MFC
	//str = "MFCTest\r\nLine2";
	CTooltipManager::CreateToolTip(_pToolTipCtrl, this, tooltiptype); //AFX_TOOLTIP_TYPE_DOCKBAR   
	_pToolTipCtrl->ModifyStyle(0, TTS_ALWAYSTIP, 0);
	//RECT r;
	//GetClientRect(&r);
	_pToolTipCtrl->AddTool(this, str);//, &r, (UINT_PTR)this->GetSafeHwnd());	// chg str to LPSTR_TEXTCALLBACK -> get TTN_NEEDTEXT  back, (UINT_PTR)hwnd
	//CWnd* oldwnd = _pToolTipCtrl->SetParent(this);
	///_pToolTipCtrl->Activate(TRUE);

//	OnUpdateToolTips((WPARAM)AFX_TOOLTIP_TYPE_ALL, 0);
	////CBCGPToolTipParams params;// = new CMFCToolTipInfo();
	////params.m_bVislManagerTheme = TRUE;
	////params.m_bBoldLabel = TRUE;
	////params.m_bDrawDescription = TRUE;
	////params.m_bDrawIcon = TRUE;
	////params.m_bRoundedCorners = TRUE;
	////params.m_bDrawSeparator = FALSE;
	////params.m_clrFillGradient = RGB(158, 158, 170);	// start grad color
	////params.m_clrFill = RGB(255, 255, 255);			// end grad color
	////params.m_clrText = RGB(61, 83, 80);
	////params.m_clrBorder = RGB(144, 149, 168);
	////params.m_nMaxDescrWidth = 400;
	////CBCGPToolTipCtrl* pToolTipEx = DYNAMIC_DOWNCAST(CBCGPToolTipCtrl, _pToolTipCtrl);
	////pToolTipEx->SetParams(&params);
	////pToolTipEx->SetFixedWidth(250, 250);
	////pToolTipEx->Update();
	////


}
/**
*
*
*
*/
void CNetworkView::StartMouseTrack(long x, long y)
{
	if (!_bEnableToolTips) return;

	if (_bDisplayingToolTip)
	{
		POINT pt{ x, y };
		::ClientToScreen(this->GetSafeHwnd(), &pt);
		if ((abs(pt.x - _trackPT.x) > 2) || (abs(pt.y - _trackPT.y) > 2))
		{
			RemoveTrackingToolTip();
		}
		return;
	}

	if ((!_bTrackingMouse) && (!_bRightButtonDown) && (!_bLeftButtonDown))// && _bEnableToolTips)
	{
		OutputDebugString(L"\n\t**StartMouseTrack");
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.hwndTrack = this->GetSafeHwnd();
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = 500;
		TrackMouseEvent(&tme);
		_bTrackingMouse = true;
		
	}
}
/**
*
*
*
*/
void CNetworkView::CancelMouseTrack()
{
	
	if (_bDisplayingToolTip)
	{
		RemoveTrackingToolTip();
		//return;
	}
	OutputDebugString(L"\n\t**CancelMouseTrack");

	//TRACKMOUSEEVENT tme;
	//tme.cbSize = sizeof(TRACKMOUSEEVENT);
	//tme.hwndTrack = this->GetSafeHwnd();
	//tme.dwFlags = TME_CANCEL | TME_LEAVE | TME_HOVER;
	//tme.dwHoverTime = 500;
	//TrackMouseEvent(&tme);


	///::SendMessage(_hwndTrackingToolTip, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)&_toolInfo);
	_bTrackingMouse = false;
}
/**
*
*
*
*/
void CNetworkView::DisplayTrackingToolTip(long x, long y)
{
	//if (!_bEnableToolTips) return;

	if (_bDisplayingToolTip) return;

	OutputDebugString(L"\n\t**DisplayTrackingToolTip");
	_trackPT = { x, y };
	////::SendMessage(_hwndTrackingToolTip, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(_trackPT.x + 12, _trackPT.y));
	////::SendMessage(_hwndTrackingToolTip, TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)&_toolInfo);
	////::SendMessage(_hwndTrackingToolTip, TTM_SETTOOLINFO, 0, (LPARAM)&_toolInfo);
	////::SendMessage(_hwndTrackingToolTip, TTM_POPUP, 0, 0);

	// MFC
	//_pToolTipCtrl->Set
	//_pToolTipCtrl->Activate(TRUE);
	//_pToolTipCtrl->SetToolInfo(&_toolInfo);
	//_pToolTipCtrl->UpdateTipText(L"Title", this);
	//_pToolTipCtrl->Update();
	//_pToolTipCtrl->EnableTrackingToolTips();
	
	//_pToolTipCtrl->Popup();
	
	//// mfc with sendmessage
	//::SendMessage(_pToolTipCtrl->GetSafeHwnd(), TTM_SETTITLE, (WPARAM)TTI_INFO, (LPARAM)L"DTC-TBONE-P1");

	//_pToolTipCtrl->SetTitle(TTI_INFO, L"DTC-TBONE-P1");
	///_pToolTipCtrl->UpdateTipText(L"DTC-TBONE-P1", this);

	::std::memset(&_toolInfo, 0, sizeof(TOOLINFO));
	_toolInfo.cbSize = sizeof(TOOLINFO);
	//_toolInfo.uFlags = TTF_IDISHWND | TTF_TRACK | TTF_ABSOLUTE;
	_toolInfo.hwnd = this->GetSafeHwnd();
	//_toolInfo.hinst = AfxGetApp()->m_hInstance;
	//_toolInfo.lpszText = L"1stline\r\n2ndline\r\n3rdline";
	_toolInfo.uId = (UINT_PTR)this->GetSafeHwnd();;




	/*TOOLINFO TI;
	::std::memset(&TI, 0, sizeof(TOOLINFO));
	TI.cbSize = sizeof(TOOLINFO);
	
	CTooltipManager::SetTooltipText(&TI, _pToolTipCtrl, tooltiptype, L"text", L"desc");*/

	CMFCToolTipCtrl* pToolTipEx = DYNAMIC_DOWNCAST(CMFCToolTipCtrl, _pToolTipCtrl);
	//pToolTipEx->UpdateTipText(L"DTC-TBONE-P1", this);		// basically sets the tooltipctrl window text, which is used by drawlabel
	//pToolTipEx->SetDescription(CString(L"L3 Devices\r\n4 Circuits\r\nOSPF Cost [3/3]\r\nBGP AS[45634]"));		// used by drawdescription

	pToolTipEx->UpdateTipText(_tooltipTitle.c_str(), this);		// basically sets the tooltipctrl window text, which is used by drawlabel
	pToolTipEx->SetDescription(_tooltipDescription.c_str());		// used by drawdescription
	//str = L"newstring";
	//_pToolTipCtrl->Popup();
	//pToolTipEx->SendMessage(TTM_ACTIVATE, TRUE);
	//pToolTipEx->Popup();

	::SendMessage(pToolTipEx->GetSafeHwnd(), TTM_SETTITLE, (WPARAM)TTI_INFO, (LPARAM)L"title text");
	//::SendMessage(pToolTipEx->GetSafeHwnd(), TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(_trackPT.x + 12, _trackPT.y));
	::SendMessage(pToolTipEx->GetSafeHwnd(), TTM_TRACKACTIVATE, (WPARAM)TRUE, (LPARAM)&_toolInfo);
	::SendMessage(pToolTipEx->GetSafeHwnd(), TTM_POPUP, 0, 0);

	/////////////


	_bDisplayingToolTip = true;
	_bTrackingMouse = false;

}
/**
*
*
*
*/
void CNetworkView::RemoveTrackingToolTip()
{
	
	if (!_bDisplayingToolTip) return;

	OutputDebugString(L"\n\t**RemoveTrackingToolTip");

	// MFC

	::SendMessage(_pToolTipCtrl->GetSafeHwnd(), TTM_POP, 0, 0);

	//_pToolTipCtrl->Pop();
	
	//::SendMessage(_hwndTrackingToolTip, TTM_POP, 0, 0);
	//::SendMessage(_hwndTrackingToolTip, TTM_TRACKACTIVATE, (WPARAM)FALSE, (LPARAM)&_toolInfo);
	_bTrackingMouse = false;

	_bDisplayingToolTip = false;
	_trackPT = { -1,-1 };

	_tooltipTitle.clear();
	_tooltipDescription.clear();
}
/**
*
*
*
*/
LRESULT CNetworkView::OnUpdateToolTips(WPARAM wp, LPARAM)
{
	UINT nTypes = (UINT)wp;

	if (_pToolTipCtrl->GetSafeHwnd() == NULL)
	{
		return 0;
	}

	if (nTypes & tooltiptype)
	{
		/*CString str;
		_pToolTipCtrl->GetText(str, this);*/

		CTooltipManager::CreateToolTip(_pToolTipCtrl, this, tooltiptype);

		if (_pToolTipCtrl != NULL)
		{
			_pToolTipCtrl->AddTool(this, str);
		}
	}


	//CMFCToolTipInfo* params = new CMFCToolTipInfo();
	//params->m_bVislManagerTheme = TRUE;
	//params->m_bBoldLabel = TRUE;
	//params->m_bDrawDescription = TRUE;
	//params->m_bDrawIcon = TRUE;
	//params->m_bRoundedCorners = TRUE;
	//params->m_bDrawSeparator = FALSE;
	//params->m_clrFillGradient = RGB(158, 158, 170);	// start grad color
	//params->m_clrFill = RGB(255, 255, 255);			// end grad color
	//params->m_clrText = RGB(61, 83, 80);
	//params->m_clrBorder = RGB(144, 149, 168);
	//params->m_nMaxDescrWidth = 400;
	//CMFCToolTipCtrl* pToolTipEx = DYNAMIC_DOWNCAST(CMFCToolTipCtrl, _pToolTipCtrl);
	//pToolTipEx->SetParams(params);
	//pToolTipEx->Update();

	return 0;
}
/**
*
*
*
*/
void CNetworkView::SetToolTipTitle(::std::wstring titleString)
{
	_tooltipTitle = titleString;
}
/**
*
*
*
*/
void CNetworkView::SetToolTipDescription(::std::wstring titleDescription)
{
	_tooltipDescription = titleDescription;
}

