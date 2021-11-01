#pragma once

#include <memory>
#include <string>
#include "NetworkView.h"
//#include "IDataCache.h"

class CNetworkView;


namespace NM
{
	namespace NetGraph
	{
		class NetworkViewSpatialHash;
		class BitmapCache;

		class BaseLayer
		{
		public:
			BaseLayer();
			virtual ~BaseLayer();
			/* Called by NetworkView */
			virtual HRESULT OnRender(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> &pRenderTarget, bool bRedraw) = 0;
			/* called by OnMessage to process windows message for dervied class*/
			virtual LRESULT ProcessMessage(UINT, WPARAM, LPARAM) = 0;
			virtual BOOL	ProcessCommandMessage(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
			virtual bool ShouldRender() = 0;
			virtual ::std::wstring GetLayerText() = 0;
			/* called by network view to get the current bitmap for rendering */
			//virtual HRESULT	GetBitmap(ID2D1Bitmap1** pBitmap) = 0;
			/* called by network view after class instantiation*/
			bool OnInitialise(CNetworkView* pView);

		protected:
			/* called by derived class to inform networkview rendertarget has been updated, usually as a result of data update not through OnRender*/
			void Present();
			/* called by derived class to get the RenderTarget for its layer - do not store pointer*/
			HRESULT GetRenderTarget(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> &pRenderTarget);
			LRESULT SendLayerMessage(UINT, WPARAM, LPARAM);			
			HWND GetHWND();
			::std::shared_ptr<NetworkViewSpatialHash> GetSpatialHashPtr();
			::std::shared_ptr<BitmapCache> GetBitmapCachePtr();
			bool IsControlKeyPressed();
			bool IsShiftKeyPressed();
			void SetToolTipTitle(::std::wstring&);
			void SetToolTipDescription(::std::wstring&);

			//::std::unique_ptr<::NM::NetGraph::IDataCache> GetDataCacheInterface();


		private:
			CNetworkView* _pView;
			/* called by networkview to process windows message, passes to ProcessMessage for derived class to deal with if need be*/ 
			LRESULT OnMessage(UINT, WPARAM, LPARAM);
			/* has instance been initialised*/
			bool	bInitialised;

			/* called from OnInitialise for derived class to initialise*/
			virtual bool Initialise() = 0;
		};

		// ns

		template<class Interface>
		inline void SafeRelease(
			Interface **ppInterfaceToRelease
			)
		{
			if (*ppInterfaceToRelease != nullptr)
			{

				(*ppInterfaceToRelease)->Release();

				(*ppInterfaceToRelease) = nullptr;
			}
		}
	}
}

