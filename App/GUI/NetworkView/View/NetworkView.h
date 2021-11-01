#pragma once
#include <memory>
#include <map>
#include <vector>
#include <set>
#include "Resource.h"

// Direct 2d
#include <d2d1.h>
#include <d2d1_1.h>
#include <D2d1_1helper.h>
#include <d2d1effects.h>
#include <d2d1effecthelpers.h>
// d3d
#include <D3D11.h>
// DXGI
#include <DXGI1_2.h>
// Direct Write
#include <dwrite_3.h>
// ??
#include <wincodec.h>
#include <wrl.h>

#include <comdef.h>

#include "BaseLayer.h"
#include "BreadCrumb.h"
//#include "DataCache.h"
//#include "IDataCache.h"
#include "Groups.h"


#include "Interfaces\ObjectDatabaseDefines.h"

namespace NM
{
	namespace ODB
	{
		class IObjectDatabase;
	}
	namespace NetGraph
	{
		class BaseLayer;
		class NetworkViewSpatialHash;
		class BitmapCache;
	}
};


class GroupViewMetadata;

#define NVLM_LOSTFOCUS			WM_USER+1			// relative to layer 
#define NVLM_HAVEFOCUS			WM_USER+2			// relative to layer
#define NVLM_GROUPDISPLAYCHANGE WM_USER+3			// group display uid changed
#define NVLM_GROUPDISPLAYSET	WM_USER+4			// group display uid set
#define NVLM_GROUPDISPLAYGET	WM_USER+5			// group display uid get

class CNetworkView : public CView
{
	struct compareGuid
	{
		bool operator()(const ::NM::ODB::ODBUID& guid1, const::NM::ODB::ODBUID& guid2) const
		{
			if (guid1.Data1 != guid2.Data1) {
				return guid1.Data1 < guid2.Data1;
			}
			if (guid1.Data2 != guid2.Data2) {
				return guid1.Data2 < guid2.Data2;
			}
			if (guid1.Data3 != guid2.Data3) {
				return guid1.Data3 < guid2.Data3;
			}
			for (int i = 0;i<8;i++) {
				if (guid1.Data4[i] != guid2.Data4[i]) {
					return guid1.Data4[i] < guid2.Data4[i];
				}
			}
			return false;
		}
	};

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	HRESULT GetRenderTarget(::NM::NetGraph::BaseLayer* pLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> &pRenderTarget);
	void	RenderUpdate(::NM::NetGraph::BaseLayer*);
	void	OnInitialUpdate();
	bool	CreateClient(::NM::NetGraph::BaseLayer*);
	LRESULT SendLayerMessage(UINT, WPARAM, LPARAM);
	bool	SetGroupView(::NM::ODB::OBJECTUID groupUID);
	::std::shared_ptr<::NM::NetGraph::NetworkViewSpatialHash> GetSpatialHashPtr();
	::std::shared_ptr<::NM::NetGraph::BitmapCache> GetBitmapCachePtr();

	void SetToolTipTitle(::std::wstring);
	void SetToolTipDescription(::std::wstring);
protected:
	CNetworkView();           // protected constructor used by dynamic creation
	virtual ~CNetworkView();


private:
	// Direct 2d
	::Microsoft::WRL::ComPtr<ID2D1Factory1>				m_pDirect2dFactory;
	::Microsoft::WRL::ComPtr<IDWriteFactory1>			m_pDirectWriteFactory;
	::Microsoft::WRL::ComPtr<IWICImagingFactory2>		m_pWICFactory;
	::Microsoft::WRL::ComPtr<ID2D1DeviceContext>		m_pDeviceContext;
	::Microsoft::WRL::ComPtr<ID2D1Device>				m_pDirect2DDevice;				// Direct2D device	
	// Direct 3d
	::Microsoft::WRL::ComPtr<ID3D11Device>				m_pDirect3DDevice;				// Direct3D device	
	::Microsoft::WRL::ComPtr<IDXGIDevice1>				m_pDxgiDevice;					// The Direct3D create device functions return a Direct3D device object. 	
	::Microsoft::WRL::ComPtr<IDXGISwapChain1>			m_pSwapChain;					// DXGI swap chain
	::Microsoft::WRL::ComPtr<ID3D11DeviceContext>		m_p3DContext;
	// D2D Bitmaps
	::Microsoft::WRL::ComPtr<ID2D1Bitmap1>				m_pBackBufferBitmap;			// Direct2D target rendering bitmap // (linked to DXGI back buffer which is linked to Direct3D pipeline)
	::Microsoft::WRL::ComPtr<ID2D1RenderTarget>			m_pTarget;
	::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget >	m_pCompatTarget;
	// Layer Text
	::Microsoft::WRL::ComPtr<IDWriteTextFormat>			m_pTextFormat;

	typedef ::std::map<::NM::NetGraph::BaseLayer*, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> >	LAYER_TO_TARGET_MAP;
	LAYER_TO_TARGET_MAP	m_pLayerTargetMap;

	typedef ::std::set<::NM::NetGraph::BaseLayer*> LAYER_UPDATE_Q;
	LAYER_UPDATE_Q m_pLayerUpdateQ;
	
	typedef ::std::vector<::NM::NetGraph::BaseLayer*> LAYER_RENDER_ORDER;
	LAYER_RENDER_ORDER _layerRenderOrder;

	typedef ::std::map<::NM::ODB::OBJECTUID, ::std::shared_ptr<GroupViewMetadata>, compareGuid> GROUP_METADATA_MAP;
	GROUP_METADATA_MAP _groupMetaData;

	Groups groups;

	int		_breadcrumbHeight;
	int		_scrollX;
	int		_scrollY;
	float	_scaleX;
	float	_scaleY;
	float	_DPIScaleX;
	float	_DPIScaleY;
	bool	_bRedraw;
	bool	_bClientRenderUpdateRequired;
	bool	_bRightButtonDown;
	bool	_bLeftButtonDown;
	bool	_bTrackingMouse;
	bool	_bDisplayingToolTip;
	bool	_bEnableToolTips;
	TOOLINFO _toolInfo;
	POINT	_trackPT;

	UINT tooltiptype;
	CToolTipCtrl* _pToolTipCtrl;
	//CMFCToolTipCtrl* _pMFCToolTipCtrl;

	::std::unique_ptr<BreadCrumb> _breadcrumb;

	CString str;
	HWND	_hwndTrackingToolTip;
	::std::wstring _tooltipTitle;
	::std::wstring _tooltipDescription;

	D2D1_POINT_2F												_lastMousePosition;
	::NM::NetGraph::BaseLayer*									_pLastLayerMessageProcessed;
	::NM::ODB::OBJECTUID										_currentLayerUID;
	::std::unique_ptr<::NM::ODB::IObjectDatabase>				_odb;					// interface to the database
	::std::shared_ptr<::NM::NetGraph::NetworkViewSpatialHash>	_spatialHash;
	::std::shared_ptr<::NM::NetGraph::BitmapCache>				_bitmapCache;

	
	HRESULT CreateDirectXResources();
	HRESULT CreateDiscardableResources();
	HRESULT CreateDiscardableResourcesEx();
	HRESULT CreateDiscardableLayerBitmaps();
	HRESULT DiscardDeviceResources();
	HRESULT OnRender();
	void	GetViewMargins(RECT &margins);
	bool	CreateRegistryService();
	bool	CreateClientCache(::NM::NetGraph::BaseLayer*, int layerRenderPosition);
	void	TransformPoints(POINT &p);
	void	UpdateRenderTargetTransforms();
	void	SetTranslation(int x, int y, bool absolute = false);
	void	SetScale(float scale, bool absolute = false);
	
	void	SaveGroupMetaData();
	void	LoadGroupMetaData();
	void	CreateTrackingToolTip();
	void    StartMouseTrack(long x, long y);
	void    CancelMouseTrack();
	void	DisplayTrackingToolTip(long x, long y);
	void	RemoveTrackingToolTip();


	LRESULT MessagePipeLine(UINT message, WPARAM wParam, LPARAM lParam, bool SendToAllClients = false);

	bool bHaveFocus;



#pragma region MFCR
public:
	virtual void OnDraw(CDC* pDC) { CDocument* pDoc = GetDocument(); };      // overridden to draw this view
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	LRESULT OnUpdateToolTips(WPARAM, LPARAM);
	DECLARE_MESSAGE_MAP()

private:
	DECLARE_DYNCREATE(CNetworkView)

#pragma endregion MFCR


	//test 
	//CComPtr<IDXGIAdapter> dxgiAdapter;
	//CComPtr<IDXGIFactory2> dxgiFactory;
	//CComPtr<ID3D11Texture2D> backBuffer;
	//::Microsoft::WRL::ComPtr<IDXGISurface> dxgiBackBuffer;


};


