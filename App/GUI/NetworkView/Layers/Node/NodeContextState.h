#pragma once
#include "NodeLayerState.h"

namespace NM
{
	namespace NetGraph
	{
		class NodeContextState :
			public NodeLayerState
		{
		public:
			NodeContextState();
			virtual ~NodeContextState();
			LRESULT ProcessMessage(UINT, WPARAM, LPARAM, NodeLayer*, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& Redraw);
			void    OnRender(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget);


		private:

		};

	}
}

