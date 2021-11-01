#include "stdafx.h"
#include "GuiTest3.h"
#include "NetworkView.h"

/**
*
*
*
*/
HRESULT CNetworkView::CreateDirectXResources()
{
	HRESULT hr = S_OK;

	// create d2d factory
	D2D1_FACTORY_OPTIONS options;
	ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));
#ifdef DEBUG
	options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif
	hr = ::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), &options, reinterpret_cast<void **>(m_pDirect2dFactory.GetAddressOf()));

	return hr;

}
/**
*
*
* Notes: https://msdn.microsoft.com/en-us/library/windows/desktop/hh780339(v=vs.85).aspx
*/
HRESULT CNetworkView::CreateDiscardableResources()
{
	HRESULT hr = S_OK;

	// This flag adds support for surfaces with a different color channel ordering than the API default.
	// You need it for compatibility with Direct2D.
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
	#ifdef DEBUG 
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

	// This array defines the set of DirectX hardware feature levels this app  supports.
	// The ordering is important and you should  preserve it.
	// Don't forget to declare your app's minimum required feature level in its
	// description.  All apps are assumed to support 9.1 unless otherwise stated.
	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	hr = D3D11CreateDevice(
		NULL,						 // specify null to use the default adapter
		D3D_DRIVER_TYPE_HARDWARE, 
		NULL,
		creationFlags,              // optionally set debug and Direct2D compatibility flags
		featureLevels,              // list of feature levels this app can support
		ARRAYSIZE(featureLevels),   // number of possible feature levels
		D3D11_SDK_VERSION,
		&m_pDirect3DDevice,                    // returns the Direct3D device created
		NULL, //featureLevels,            // returns feature level of device created
		&m_p3DContext                    // returns the device immediate context
		);
	if (FAILED(hr))
		return hr;

	hr = m_pDirect3DDevice.As(&m_pDxgiDevice);
	if (FAILED(hr))
		return hr;

	hr = CreateDiscardableResourcesEx();
	if (FAILED(hr))
		return hr;
	
	return hr;
}
/**
*
*
*
*/
HRESULT CNetworkView::CreateDiscardableResourcesEx()
{
	HRESULT hr = S_OK;
	DiscardDeviceResources();
	RECT rcClient;
	GetClientRect(&rcClient);
	UINT height = rcClient.bottom - _breadcrumbHeight;

	// Allocate a descriptor.
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
	swapChainDesc.Width = 0;                           // use automatic sizing
	swapChainDesc.Height = height; // 0;
	swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // this is the most common swapchain format
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1;                // don't use multi-sampling
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;                     // use double buffering to enable flip
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // all apps must use this SwapEffect
	swapChainDesc.Flags = 0;

	// Identify the physical adapter (GPU or card) this device runs on.
	CComPtr<IDXGIAdapter> dxgiAdapter;
	hr = m_pDxgiDevice->GetAdapter(&dxgiAdapter);

	// Get the factory object that created the DXGI device.
	CComPtr<IDXGIFactory2> dxgiFactory;
	hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));

	// Get the final swap chain for this window from the DXGI factory.
	hr = dxgiFactory->CreateSwapChainForHwnd(m_pDirect3DDevice.Get(), this->GetSafeHwnd(), /*reinterpret_cast<IUnknown*>()*/ &swapChainDesc, nullptr, nullptr, &m_pSwapChain);

	// Ensure that DXGI doesn't queue more than one frame at a time.
	hr = m_pDxgiDevice->SetMaximumFrameLatency(1);

	// Get the backbuffer for this window which is be the final 3D render target.
	CComPtr<ID3D11Texture2D> backBuffer;
	hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));

	// Now we set up the Direct2D render target bitmap linked to the swapchain. 
	// Whenever we render to this bitmap, it will be directly rendered to the 
	// swapchain associated with the window.
	D2D1_BITMAP_PROPERTIES1 bitmapProperties =
		D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
			_DPIScaleX,
			_DPIScaleY
		);

	// Direct2D needs the dxgi version of the backbuffer surface pointer.
	::Microsoft::WRL::ComPtr<IDXGISurface> dxgiBackBuffer;
	hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));

	hr = m_pDirect2dFactory->CreateDevice(m_pDxgiDevice.Get(), &m_pDirect2DDevice);
	//D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS, app is single threaded, more efficient NOT to use multithread option	
	hr = m_pDirect2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pDeviceContext);
	// Get a D2D surface from the DXGI back buffer to use as the D2D render target.
	hr = m_pDeviceContext->CreateBitmapFromDxgiSurface(dxgiBackBuffer.Get(), &bitmapProperties, m_pBackBufferBitmap.GetAddressOf());
	// So now we can set the Direct2D render target.
	m_pDeviceContext->SetTarget(m_pBackBufferBitmap.Get());

	ASSERT(!FAILED(hr));
	hr = CreateDiscardableLayerBitmaps();
	ASSERT(!FAILED(hr));
	return hr;
}
/*
*
*
*
*/
HRESULT CNetworkView::CreateDiscardableLayerBitmaps()
{
	HRESULT hr = S_OK;

	// Create Client Layers Compat. Render Targets
	// UINT dpi = GetDpiForWindow(this->GetSafeHwnd());
	// DPI Check/Calc Needed if we are to be DPI aware
	// DIPs = pixels / (DPI/96.0)
	
	D2D1_SIZE_F sizeF = m_pDeviceContext->GetSize();
	sizeF.height = max(0, sizeF.height - _breadcrumbHeight);
	sizeF.width = max(0, sizeF.width);

	LAYER_TO_TARGET_MAP::iterator it = m_pLayerTargetMap.begin();
	while (it != m_pLayerTargetMap.end())
	{
		ASSERT(it->second == nullptr);
		hr = m_pDeviceContext->CreateCompatibleRenderTarget(sizeF, it->second.GetAddressOf());
		ASSERT(!FAILED(hr));

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

		it->second->SetTransform(translation * scale);
		++it;
	}

	_bRedraw = true;
	ASSERT(!FAILED(hr));
	return hr;
}
/**
* DiscardDeviceResources
*
* Just release stuff here that will need to be rebuilt
* on a WM_SIZE message.
*
* As far as what requires changing when the device itself changes ?!?!?, commented out
* lines below are not part of CreateDiscardableResourcesEx(), only active lines are, the commented
* ones are in CreateDiscardableResources()
*/
HRESULT CNetworkView::DiscardDeviceResources()
{
	HRESULT hr = S_OK;
	//OutputDebugString(L"\nCOMPAT: DiscardDeviceResources");

	LAYER_TO_TARGET_MAP::iterator it = m_pLayerTargetMap.begin();
	while (it != m_pLayerTargetMap.end())
	{
		it->second.Reset();
		++it;
	}

	m_pSwapChain.Reset();
	m_pBackBufferBitmap.Reset();
	m_pDeviceContext.Reset();
	m_pDirect2DDevice.Reset();
	_bRedraw = true;
	return hr;
}