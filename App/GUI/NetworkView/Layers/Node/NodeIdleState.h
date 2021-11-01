#pragma once
#include "NodeLayerState.h"


namespace NM
{
	namespace NetGraph
	{

		class NodeIdleState :
			public NodeLayerState
		{
		public:
			NodeIdleState();
			virtual ~NodeIdleState();
			LRESULT ProcessMessage(UINT, WPARAM, LPARAM, NodeLayer*, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& Redraw);
			void    OnRender(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget);

		private:
			POINT _lastMousePoint;
			bool  _lastLeftButtonStateDown;

			LRESULT OnLeftButtonDown(UINT msg, WPARAM wParam, LPARAM lParam, NodeLayer* pNodeLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& Redraw);
			LRESULT OnLeftButtonUp(UINT msg, WPARAM wParam, LPARAM lParam, NodeLayer* pNodeLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& Redraw);
		};

	}
}

