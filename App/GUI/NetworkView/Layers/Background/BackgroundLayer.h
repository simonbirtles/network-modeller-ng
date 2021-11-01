#pragma once
#include "BaseLayer.h"


namespace NM
{
	namespace NetGraph
	{

		class BackgroundLayer : public BaseLayer
		{
		public:
			BackgroundLayer();
			~BackgroundLayer();

			HRESULT OnRender(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> &pRenderTarget, bool bRedraw);
			bool ShouldRender() { return true; };
			LRESULT ProcessMessage(UINT, WPARAM, LPARAM);
			BOOL	ProcessCommandMessage(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
			bool Initialise();
			::std::wstring GetLayerText() { return L"Background Layer"; }

		private:
			::Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> _pGridBrush;
			IDWriteFactory* pDWriteFactory_;
			::Microsoft::WRL::ComPtr<IDWriteTextFormat> pTextFormat_;
			const wchar_t* wszText_;
			UINT32 cTextLength_;
			::Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> _pTextWhiteBrush;
			//::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> _pRenderTarget;										// the last render target passed to us
			int _spatialGridSize;
			D2D1_COLOR_F _gridLineColor;
			bool _bLeftButtonDown;
			bool _bDrawGridLines;
			


		};

		// ns
	}
}

