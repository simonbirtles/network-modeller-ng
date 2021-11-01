#include "stdafx.h"
#include "LinkDrawResources.h"

namespace NM
{
	namespace NetGraph
	{

		LinkDrawResources::LinkDrawResources()
		{
		}


		LinkDrawResources::~LinkDrawResources()
		{
			_nodebrushSelected.Reset();
			_nodebrushControlPtsLine.Reset();
			_nodebrushRed.Reset();
			///_nodebrushNormal.Reset();
			_nodebrushStateDown.Reset();
			_nodebrushStateUnknown.Reset();
		}
		/**
		*
		*
		*
		*/

		void LinkDrawResources::RefreshContent(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> &pRenderTarget)
		{
			_nodebrushSelected.Reset();
			_nodebrushControlPtsLine.Reset();
			_nodebrushRed.Reset();
			//c = _nodebrushNormal.Reset();
			_nodebrushStateDown.Reset();
			_nodebrushStateUnknown.Reset();

			//pRenderTarget->CreateSolidColorBrush(_linkUserColorNormalState, _nodebrushNormal.GetAddressOf());
			pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0.5f, 0.5f, 1.0f), _nodebrushRed.GetAddressOf());
			pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.5f, 0.5f, 0.5f, 1.0f), _nodebrushControlPtsLine.GetAddressOf());
			pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 0.0f, 0.5f), _nodebrushSelected.GetAddressOf());
			pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkCyan), _nodebrushStateUnknown.GetAddressOf());
			pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkGray), _nodebrushStateDown.GetAddressOf());

			return;
		}

	}
}