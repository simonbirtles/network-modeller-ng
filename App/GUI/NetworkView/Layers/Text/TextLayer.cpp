#include "stdafx.h"
#include "TextLayer.h"
#include "IServiceRegistry.h"							// registry interface
#include "Interfaces\IObjectDatabase.h"					// for client interface to core database 
#include "Interfaces\IAttribute.h"						// interface to an attribute copy
#include "Interfaces\IDatabaseUpdate.h"					// for client interface for core database update notifications
//#include "Interfaces\IAdjacencyMatrix.h"

extern ::NM::Registry::IServiceRegistry* reg;

using ::NM::ODB::OBJECTUID;
using ::NM::ODB::UPVALUE;
using ::NM::ODB::INVALID_OBJECT_UID;
using ::NM::ODB::real_int;
using ::NM::ODB::real_string;


namespace NM
{
	namespace NetGraph
	{
		/**
		*
		*
		*
		*/
		TextLayer::TextLayer()
		{
		}
		/**
		*
		*
		*
		*/
		TextLayer::~TextLayer()
		{
			//pTextWhiteBrush_->Release();
		}
		/**
		*
		*
		*
		*/
		bool TextLayer::Initialise()
		{
			OutputDebugString(L"\nTextLayer::Initialise");

			if (!reg)
				return false;

			// Get The Database Handle
			_odb.reset(reinterpret_cast<NM::ODB::IObjectDatabase*>(reg->GetClientInterface(L"ODB")));
			if (!_odb)
				return false;


			// create direct write factory
			HRESULT hr = DWriteCreateFactory(
				DWRITE_FACTORY_TYPE_SHARED,
				__uuidof(IDWriteFactory),
				reinterpret_cast<IUnknown**>(&pDWriteFactory_)
				);

			wszText_ = L"Text Layer";
			cTextLength_ = (UINT32)wcslen(wszText_);

			if (SUCCEEDED(hr))
			{
				hr = pDWriteFactory_->CreateTextFormat(
					L"Gabriola",                // Font family name.
					NULL,                       // Font collection (NULL sets it to use the system font collection).
					DWRITE_FONT_WEIGHT_REGULAR,
					DWRITE_FONT_STYLE_NORMAL,
					DWRITE_FONT_STRETCH_NORMAL,
					36.0f,
					L"en-us",
					&pTextFormat_
					);
			}

			//if (SUCCEEDED(hr))
			//{
			//	hr = pDWriteFactory_->CreateTextFormat(
			//		L"Calibri",                // Font family name.
			//		NULL,                       // Font collection (NULL sets it to use the system font collection).
			//		DWRITE_FONT_WEIGHT_REGULAR,
			//		DWRITE_FONT_STYLE_NORMAL,
			//		DWRITE_FONT_STRETCH_NORMAL,
			//		8.0f,
			//		L"en-us",
			//		&pTextNodeFormat_
			//		);
			//}

			// Center align (horizontally) the text.
			if (SUCCEEDED(hr))
			{
				hr = pTextFormat_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
				hr = pTextFormat_->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
				
				//hr = pTextNodeFormat_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
				//hr = pTextNodeFormat_->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
			}
			
			return true;
		}
		/**
		* OnRender
		*
		* Create/Recreate bitmap if is requires it, either way - make a usable bitmap available.
		*/
		HRESULT TextLayer::OnRender(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> &pRenderTarget, bool bRedraw)
		{
			HRESULT hr = S_OK;
			// unless forced redraw, we dont need to as its static content
			if (!bRedraw) return S_OK;
			OutputDebugString(L"\nTextLayer::OnRender");


			pRenderTarget->BeginDraw();

#ifdef DEBUG
			hr = pRenderTarget->CreateSolidColorBrush(
				D2D1::ColorF(D2D1::ColorF::White),
				&pTextWhiteBrush_
				);

			D2D1_SIZE_F size = pRenderTarget->GetSize();
			float dpiScaleX_ = 0;;
			float dpiScaleY_ = 0;
			pRenderTarget->GetDpi(&dpiScaleX_, &dpiScaleY_);

			D2D1_RECT_F layoutRect = D2D1::RectF(
				static_cast<FLOAT>(0),// / dpiScaleX_,
				static_cast<FLOAT>(-20),// / dpiScaleY_,
				static_cast<FLOAT>(size.width - 8),// / dpiScaleX_,
				static_cast<FLOAT>(size.height)// / dpiScaleY_
				);

			// transform
			::D2D1::Matrix3x2F currentTransform;
			pRenderTarget->GetTransform(&currentTransform);
			pRenderTarget->SetTransform(::D2D1::Matrix3x2F::Identity());

			pRenderTarget->DrawText(
				wszText_,        // The string to render.
				cTextLength_,    // The string's length.
				pTextFormat_.Get(),    // The text format.
				layoutRect,       // The region of the window where the text will be rendered.
				pTextWhiteBrush_.Get()     // The brush used to draw the text.
				);
			
			pRenderTarget->SetTransform(currentTransform);
#endif
			hr = pRenderTarget->EndDraw();

			return hr;
		}
		/**
		*
		*
		* An application returns zero if it processes this message.
		*/
		LRESULT TextLayer::ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam)
		{
			LRESULT lResult = 1;			
			//OutputDebugString(L"\nTextLayer::ProcessMessage");
			switch (message)
			{
			case WM_SIZE:
				break;

			default:
				break;
			}



			return lResult;
		}

		// ns
	}
}

