#pragma once
#include "BaseLayer.h"
#include <wrl.h>

namespace NM
{
	namespace ODB
	{
		class IObjectDatabase;
	}

	namespace NetGraph
	{

		class TextLayer : public BaseLayer
		{
		public:
			TextLayer();
			~TextLayer();

			HRESULT OnRender(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> &pRenderTarget, bool bRedraw);
			LRESULT ProcessMessage(UINT, WPARAM, LPARAM);
			bool ShouldRender() { return false; };
			bool Initialise();
			::std::wstring GetLayerText() { return L"Text Layer"; }

		private:
			IDWriteFactory* pDWriteFactory_;
			::Microsoft::WRL::ComPtr<IDWriteTextFormat> pTextFormat_;
			//::Microsoft::WRL::ComPtr<IDWriteTextFormat> pTextNodeFormat_;
			const wchar_t* wszText_;
			UINT32 cTextLength_;
			::Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pTextWhiteBrush_;

			::std::unique_ptr<::NM::ODB::IObjectDatabase> _odb;

		};

		// ns
	}
}

