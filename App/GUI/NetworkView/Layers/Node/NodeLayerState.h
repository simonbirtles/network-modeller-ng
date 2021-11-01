#pragma once
#include "NodeLayer.h"

namespace NM
{
	namespace NetGraph
	{
		class NodeLayer;

		class NodeLayerState
		{
		public:
			NodeLayerState();
			virtual ~NodeLayerState();
			virtual LRESULT ProcessMessage(UINT, WPARAM, LPARAM, NodeLayer*, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& Redraw) = 0;
			virtual void    OnRender(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget) = 0;

		protected:
			bool IsControlKeyPressed();
			bool IsShiftKeyPressed();
			bool IsLeftButtonPressed();

		private:

		};

	}
}

