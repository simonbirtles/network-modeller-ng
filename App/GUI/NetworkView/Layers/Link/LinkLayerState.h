#pragma once
#include <wrl.h>
#include <memory>

namespace NM
{
	namespace NetGraph
	{
		class LinkLayer;
		class LinkDrawResources;

		class LinkLayerState
		{
		public:
			LinkLayerState();
			virtual ~LinkLayerState();
			virtual LRESULT ProcessMessage(UINT, WPARAM, LPARAM, LinkLayer*, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& Redraw) = 0;
			virtual void    OnRender(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, ::std::shared_ptr<LinkDrawResources> pDrawResources) = 0;

		protected:
			bool IsControlKeyPressed();
			bool IsShiftKeyPressed();
			bool IsLeftButtonPressed();

		private:
		};

	}
}

