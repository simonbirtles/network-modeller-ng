#include "stdafx.h"
#include "BackgroundLayer.h"
#include "NetworkViewSpatialHash.h"
#include "IServiceRegistry.h"							// registry interface
#include "ISelectedObjects.h"							// holds references to currently selected objects application wide
#include <memory>

extern NM::Registry::IServiceRegistry* reg;

namespace NM
{
	namespace NetGraph
	{
		/**
		*
		*
		*
		*/
		BackgroundLayer::BackgroundLayer() :
			_spatialGridSize(64), // default until actual assigned
			_bLeftButtonDown(false),
			_bDrawGridLines(true)
			
		{
			//_gridLineColor = D2D1::ColorF(D2D1::ColorF::IndianRed);
			//_gridLineColor = D2D1::ColorF(0xBF6D6D);
			_gridLineColor = D2D1::ColorF(0xA24A4A);
		}
		/**
		*
		*
		*
		*/
		BackgroundLayer::~BackgroundLayer()
		{
		}
		/**
		*
		*
		*
		*/
		bool BackgroundLayer::Initialise()
		{
			OutputDebugString(L"\nBackgroundLayer::Initialise");

			::std::shared_ptr<::NM::NetGraph::NetworkViewSpatialHash> pSpatialHash = GetSpatialHashPtr();
			_spatialGridSize = pSpatialHash->GetSpatialGridSize();

			// create direct write factory
			HRESULT hr = DWriteCreateFactory(
				DWRITE_FACTORY_TYPE_SHARED,
				__uuidof(IDWriteFactory),
				reinterpret_cast<IUnknown**>(&pDWriteFactory_)
				);

			wszText_ = L"Background Layer";
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

			// Center align (horizontally) the text.
			if (SUCCEEDED(hr))
			{
				hr = pTextFormat_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
			}

			if (SUCCEEDED(hr))
			{
				hr = pTextFormat_->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
			}

			::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget;
			GetRenderTarget(pRenderTarget);
			ASSERT(pRenderTarget);
			OnRender(pRenderTarget, true);

			return true;
		}
		/**
		* OnRender
		*
		* Create/Recreate bitmap if is requires it, either way - make a usable bitmap available.
		*/
		HRESULT BackgroundLayer::OnRender(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> &pRenderTarget, bool bRedraw)
		{
			HRESULT hr = S_OK;
			// unless forced redraw, we dont need to as its static content
			if (!bRedraw) return S_OK;

			ASSERT(pRenderTarget);

			pRenderTarget->BeginDraw();
			pRenderTarget->Clear();

			// save current transform 
			::D2D1::Matrix3x2F currentTransform;
			pRenderTarget->GetTransform(&currentTransform);


			if (_bDrawGridLines)
			{
				// draw grid
				if (!_pGridBrush)
				{
					//::Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> _pGridBrush;
					hr = pRenderTarget->CreateSolidColorBrush(_gridLineColor, &_pGridBrush);
				}

				float gridSize = static_cast<float>(_spatialGridSize) * currentTransform._11; // or _22 but x=y for gridsize.
				D2D1_SIZE_F renderSize = pRenderTarget->GetSize();
				size_t ylines = static_cast<size_t>((renderSize.width / gridSize) + 2);
				size_t xlines = static_cast<size_t>((renderSize.height / gridSize) + 2);


				pRenderTarget->SetTransform(::D2D1::Matrix3x2F::Identity());

				int startx = (((int)currentTransform._31) % (int)gridSize);
				int starty = (((int)currentTransform._32) % (int)gridSize);

				//horizontal lines =
				for (size_t t = 0; t < xlines; ++t)
				{
					pRenderTarget->DrawLine(
						::D2D1::Point2F(static_cast<float>(startx), (gridSize*t) + starty),
						::D2D1::Point2F(renderSize.width, (gridSize*t) + starty),
						_pGridBrush.Get()
						);
				}

				// vertical lines ||			
				for (size_t t = 0; t < ylines; ++t)
				{
					pRenderTarget->DrawLine(
						::D2D1::Point2F(((gridSize*t) + startx), 0.0f),
						::D2D1::Point2F(((gridSize*t) + startx), renderSize.height),
						_pGridBrush.Get()
						);
				}

			}

#ifdef DEBUG	
		
			pRenderTarget->SetTransform(::D2D1::Matrix3x2F::Identity());
			// draw layer text
			D2D1_SIZE_F size = pRenderTarget->GetSize();
			float dpiScaleX_ = 0;;
			float dpiScaleY_ = 0;
			pRenderTarget->GetDpi(&dpiScaleX_, &dpiScaleY_);
			D2D1_RECT_F layoutRect = D2D1::RectF(
				static_cast<FLOAT>(0),// / dpiScaleX_,
				static_cast<FLOAT>(0),// / dpiScaleY_,
				static_cast<FLOAT>(size.width - 8),// / dpiScaleX_,
				static_cast<FLOAT>(size.height)// / dpiScaleY_
				);

			if (!_pTextWhiteBrush)
			{
				hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 0.1f), &_pTextWhiteBrush);
			}

			pRenderTarget->DrawText(
				wszText_,        // The string to render.
				cTextLength_,    // The string's length.
				pTextFormat_.Get(),    // The text format.
				layoutRect,       // The region of the window where the text will be rendered.
				_pTextWhiteBrush.Get()     // The brush used to draw the text.
				);
						
#endif
			// restore transform back to previous in case its changed in above code.
			pRenderTarget->SetTransform(currentTransform);
			hr = pRenderTarget->EndDraw();

			return hr;
		}
		/**
		*
		*
		* An application returns zero if it processes this message.
		*/
		LRESULT BackgroundLayer::ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam)
		{
			LRESULT lResult = 1;			
			switch (message)
			{
			case WM_SIZE:
			{
				_pGridBrush.Reset();
				_pTextWhiteBrush.Reset();
				::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pTarget;
				GetRenderTarget(pTarget);
				ASSERT(pTarget);
				OnRender(pTarget, true);
			}
				break;

			case WM_LBUTTONDOWN:
				// nothing to do
				_bLeftButtonDown = true;
				break;

			case WM_LBUTTONUP:
				// if ctrl key not pressed, then de select all current selections
				if( (_bLeftButtonDown) && (!IsControlKeyPressed()))
				{
					// if the global registry is not available we are in trouble
					if (!reg)
						throw ::std::runtime_error("Application Registry Not Available, Cannot Continue.");

					::std::unique_ptr<NM::ISelectedObjects> selected;
					selected.reset(reinterpret_cast<NM::ISelectedObjects*>(reg->GetClientInterface(L"SELECTEDOBJECTS")));

					if (!selected)
						throw ::std::runtime_error("Selected Objects Service Not Available, Cannot Continue.");

					selected->Clear();
					lResult = 0;
				}					
				_bLeftButtonDown = false;
				break;

			default:
				break;
			}

			return lResult;
		}
		/**
		* ProcessCommandMessage
		*
		* Any command messages from Menu buttons or popupmenus etc
		*
		* Return TRUE if message was processed
		*/
		BOOL BackgroundLayer::ProcessCommandMessage(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
		{
			BOOL bProcessed = FALSE;
			CCmdUI* pCmdUI = (CCmdUI*)pExtra;

			switch (nID)
			{
			case ID_TOGGLE_GRID:
			{
				//OutputDebugString(L"\n\t\t ID_BUTTON_TIPS");
				if (nCode == CN_UPDATE_COMMAND_UI)
				{
					//OutputDebugString(L"\tCN_UPDATE_COMMAND_UI");
					CCmdUI* pCmdUI = (CCmdUI*)pExtra;
					pCmdUI->Enable(TRUE);
					pCmdUI->SetCheck(_bDrawGridLines ? TRUE : FALSE);
					return TRUE;
				}

				if (nCode == CN_COMMAND)
				{
					//OutputDebugString(L"\tCN_COMMAND");
					_bDrawGridLines = !_bDrawGridLines;
					::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget;
					GetRenderTarget(pRenderTarget);
					ASSERT(pRenderTarget);
					OnRender(pRenderTarget, true);
					Present();					
					return TRUE;
				}
			}
			break;


			default:
				break;
			}

			return bProcessed;
		}

		// ns
	}
}

