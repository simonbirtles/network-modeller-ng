#include "stdafx.h"
#include "BaseLayer.h"
#include "NetworkView.h"
#include "NetworkViewSpatialHash.h"
#include "BitmapCache.h"

namespace NM
{
	namespace NetGraph
	{

		/**
		*
		*
		*
		*/
		BaseLayer::BaseLayer() :
			bInitialised(false)
		{
		}
		/**
		*
		*
		*
		*/
		BaseLayer::~BaseLayer()
		{
		}
		bool BaseLayer::OnInitialise(CNetworkView* pView)
		{
			if (bInitialised) return bInitialised;
			if (!pView)	return false;
			_pView = pView;

			// call dervied class init.
			Initialise();

			bInitialised = true;
			return bInitialised;
		}
		/**
		*
		*
		*
		*/
		HRESULT BaseLayer::GetRenderTarget(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> &pRenderTarget)
		{
			return _pView->GetRenderTarget(this, pRenderTarget);
		}
		/**
		*
		*
		*
		*/
		LRESULT BaseLayer::SendLayerMessage(UINT message, WPARAM wParam, LPARAM lParam)
		{
			return _pView->SendLayerMessage(message, wParam, lParam);
		}
		/**
		*
		*
		*
		*/
		::std::shared_ptr<NetworkViewSpatialHash> BaseLayer::GetSpatialHashPtr()
		{
			return _pView->GetSpatialHashPtr();
		}
		/**
		*
		*
		*
		*/
		::std::shared_ptr<BitmapCache> BaseLayer::GetBitmapCachePtr()
		{
			return _pView->GetBitmapCachePtr();
		}
		/**
		*
		*
		*
		*/
		HWND BaseLayer::GetHWND()
		{
			return _pView->GetSafeHwnd();
		}
		/**
		*
		*
		*
		*/
		void BaseLayer::Present()
		{
			_pView->RenderUpdate(this);
		}
		/**
		*
		*
		*
		*/
		void BaseLayer::SetToolTipTitle(::std::wstring& titleString)
		{
			_pView->SetToolTipTitle(titleString);
		}
		/**
		*
		*
		*
		*/
		void BaseLayer::SetToolTipDescription(::std::wstring& descriptionString) 
		{
			_pView->SetToolTipDescription(descriptionString);
		}
		/**
		* OnMessage
		*
		* Message pipeline through layer objects
		* An application returns zero if it processes this message.
		*/
		LRESULT BaseLayer::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			LRESULT lResult = 1;
			
			switch (uMsg)
			{
			case 0:
				break;


			default:
				break;
			}

			lResult = ProcessMessage(uMsg, wParam, lParam);
			
			return lResult;
		}
		/**
		*
		*
		*
		*/
		BOOL BaseLayer::ProcessCommandMessage(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
		{
			return FALSE;
		}
		/**
		*
		*
		*
		*/
		bool BaseLayer::IsControlKeyPressed()
		{
			return (GetKeyState(VK_CONTROL) < 0);

			/*return ((GetAsyncKeyState(VK_CONTROL) & 0x8000) == 1);*/
		}
		/**
		*
		*
		*
		*/
		bool BaseLayer::IsShiftKeyPressed()
		{
			return (GetKeyState(VK_SHIFT) < 0);

			/*		return ((GetAsyncKeyState(VK_SHIFT) & 0x8000) == 1);*/
		}
		//ns
	}
}
