#pragma once
// Direct 2d
#include <d2d1.h>
#include <d2d1_1.h>
#include <D2d1_1helper.h>
#include <d2d1effects.h>
#include <d2d1effecthelpers.h>
#include <wrl.h>
#include <set>

namespace NM
{
	namespace NetGraph
	{
		class LinkDrawResources
		{
		public:
			LinkDrawResources();
			~LinkDrawResources();

			void RefreshContent(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> &pRenderTarget);

			//::Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>	_nodebrushNormal;
			::Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>	_nodebrushRed;
			::Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>	_nodebrushControlPtsLine;
			::Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>	_nodebrushSelected;
			::Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>	_nodebrushStateDown;
			::Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>	_nodebrushStateUnknown;
		};

	}
}
