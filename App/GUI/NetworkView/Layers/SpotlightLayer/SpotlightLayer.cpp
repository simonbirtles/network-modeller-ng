#include "stdafx.h"
#include "SpotLightLayer.h"
#include "IServiceRegistry.h"							// registry interface
#include "Interfaces\IObjectDatabase.h"					// for client interface to core database 
#include "Interfaces\IAttribute.h"						// interface to an attribute copy
#include "Interfaces\IDatabaseUpdate.h"					// for client interface for core database update notifications
//#include "Interfaces\IAdjacencyMatrix.h"

#include <d2d1effects.h>
#include <d2d1effecthelpers.h>
#include <d2d1effects_2.h>

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
		SpotLightLayer::SpotLightLayer() :
			_currentLayerUID(::NM::ODB::INVALID_OBJECT_UID),
			_spotlightOn(false)
		{
		}
		/**
		*
		*
		*
		*/
		SpotLightLayer::~SpotLightLayer()
		{
		}
		/**
		*
		*
		*
		*/
		bool SpotLightLayer::Initialise()
		{
			OutputDebugString(L"\nSpotLightLayer::Initialise");

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

			wszText_ = L"Spotlight Layer";
			cTextLength_ = (UINT32)wcslen(wszText_);

			if (SUCCEEDED(hr))
			{
				hr = pDWriteFactory_->CreateTextFormat(
					L"Gabriola",                // Font family name.
					NULL,                       // Font collection (NULL sets it to use the system font collection).
					DWRITE_FONT_WEIGHT_LIGHT,
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
				hr = pTextFormat_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
				hr = pTextFormat_->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
			}

			// get factory for links to create pathgeometries
			::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget;
			GetRenderTarget(pRenderTarget);
			ASSERT(pRenderTarget);
			_pD2DFactory.Reset();
			pRenderTarget->GetFactory(_pD2DFactory.GetAddressOf());
			assert(_pD2DFactory.Get());
			
			RefreshContent();

			return true;
		}
		/**
		* OnRender
		*
		* Create/Recreate bitmap if is requires it, either way - make a usable bitmap available.
		*/
		HRESULT SpotLightLayer::OnRender(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> &pRenderTarget, bool bRedraw)
		{
			OutputDebugString(L"\nSpotLightLayer::OnRender");
			HRESULT hr = S_OK;
			// unless forced redraw, we dont need to as its static content
			if (!bRedraw) return S_OK;
			OutputDebugString(L"\tRedraw = TRUE");

			// Redraw requested, this will be due to transform change, WM_SIZE although this will be called after
			assert(_bitmapBackground.Get());
			assert(_objectList.size() > 0);
			assert(_renderBitmap.Get());

			// create the spolight bitmap
			CreateSpotlight();

			// draw to our render target
			D2D1_MATRIX_3X2_F layerTransform;
			pRenderTarget->GetTransform(&layerTransform);
			pRenderTarget->SetTransform(::D2D1::Matrix3x2F::Identity());
			pRenderTarget->BeginDraw();
			pRenderTarget->Clear();
			pRenderTarget->DrawBitmap(_renderBitmap.Get());

#ifdef DEBUG
			hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 0.2f), &pTextWhiteBrush_);
			
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
			//::D2D1::Matrix3x2F currentTransform;
			//pRenderTarget->GetTransform(&currentTransform);
			//pRenderTarget->SetTransform(::D2D1::Matrix3x2F::Identity());
			
			pRenderTarget->DrawText(
				wszText_,        // The string to render.
				cTextLength_,    // The string's length.
				pTextFormat_.Get(),    // The text format.
				layoutRect,       // The region of the window where the text will be rendered.
				pTextWhiteBrush_.Get()     // The brush used to draw the text.
				);
						
			//pRenderTarget->SetTransform(currentTransform);
#endif

			pRenderTarget->EndDraw();
			pRenderTarget->SetTransform(layerTransform);
			return hr;
		}
		/**
		*
		*
		* An application returns zero if it processes this message.
		*/
		LRESULT SpotLightLayer::ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam)
		{
			LRESULT lResult = 1;			
			//OutputDebugString(L"\nSpotLightLayer::ProcessMessage");
			switch (message)
			{
			case WM_SIZE:
				RefreshContent();
				lResult = 1;	// other layers need this although the parent window ignores this on WM_SIZE
				break;

			case WM_LBUTTONDOWN:
				if (_spotlightOn)
				{
					// do nothing but return we dealt with as we dont want it going to 
					// other layers until we remove spotlight
					lResult = 0;
				}
				break;

			case WM_LBUTTONUP:
				if (_spotlightOn)
				{
					SetSpotlightOff();
					lResult = 0;
				}
				break;

			case NVLM_GROUPDISPLAYCHANGE:
				_currentLayerUID = *(::NM::ODB::OBJECTUID*)wParam;
				lResult = 0;
				break;

			default:
				break;
			}
			
			return lResult;
		}
		/**
		* CreateSpotlight (Public Method)
		*
		*
		*/
		void SpotLightLayer::SetSpotlightOn(::std::vector<::NM::ODB::OBJECTUID>& objectList)
		{
			_objectList = objectList;

			::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget;
			HRESULT hr = GetRenderTarget(pRenderTarget);
			ASSERT(pRenderTarget);

			if (CreateSpotlight() > 0)
			{
				_spotlightOn = true;
				// render
				OnRender(pRenderTarget, true);
				Present();
			}

			return;
		}
		/**
		*
		*
		*
		*/
		void SpotLightLayer::SetSpotlightOff()
		{
			_spotlightOn = false;
			_objectList.clear();
			Present();
		}
		/**
		* CreateSpotlight
		*
		* Creates the spolight bitmap (Private)
		*/
		size_t SpotLightLayer::CreateSpotlight()
		{
			OutputDebugString(L"\nCreating Spotlight");

			assert(_objectList.size() != 0);

			size_t displayedItems = 0;

			::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget;
			HRESULT hr = GetRenderTarget(pRenderTarget);
			ASSERT(pRenderTarget);

			// get current device context from rendertarget
			::Microsoft::WRL::ComPtr<ID2D1DeviceContext> pDeviceContext = nullptr;
			hr = pRenderTarget.Get()->QueryInterface(IID_ID2D1DeviceContext, &pDeviceContext);
			ASSERT(pDeviceContext);


			// save current rendertarget
			::Microsoft::WRL::ComPtr<ID2D1Image> oldTarget;
			pDeviceContext->GetTarget(&oldTarget);
			D2D1_MATRIX_3X2_F layerTransform;
			D2D1_MATRIX_3X2_F deviceTransform;
			pRenderTarget->GetTransform(&layerTransform);
			pDeviceContext->GetTransform(&deviceTransform);
			
			// set new target as bitmap_spotlight and apply the transform from our layer to the devicecontext
			// as the spotlight must be transformed
			pDeviceContext->SetTarget(_bitmapSpotlight.Get());
			pDeviceContext->SetTransform(layerTransform);

			_spotBrush.Reset();
			// color is irrelevent, must be opaque (default)
			pDeviceContext->CreateSolidColorBrush(::D2D1::ColorF(::D2D1::ColorF::Blue), _spotBrush.GetAddressOf());

			// draw spotlights			
			::std::vector<::NM::ODB::OBJECTUID>::iterator objectit = _objectList.begin();

			pDeviceContext->BeginDraw();
			pDeviceContext->Clear();
			while (objectit != _objectList.end())
			{				
				::std::wstring objecttype = _odb->GetObjectTypeName(*objectit);
				if( (objecttype == L"vertex") || (objecttype == L"group"))
				{
					displayedItems += (DrawNodeSpotlight(pDeviceContext, *objectit) ? 1 : 0);
				}
				else if (objecttype == L"edge")
				{
					displayedItems += (DrawLinkSpotlight(pDeviceContext, *objectit) ? 1 : 0);
				}								

				++objectit;
			}
			pDeviceContext->EndDraw();
			

			// create composite of background and spotlight using source out mode
			_compositeEffect.Reset();
			pDeviceContext->CreateEffect(CLSID_D2D1Composite, &_compositeEffect);
			_compositeEffect->SetValue(D2D1_COMPOSITE_PROP_MODE, D2D1_COMPOSITE_MODE_SOURCE_OUT);
			_compositeEffect->SetInput(0, _bitmapSpotlight.Get());
			_compositeEffect->SetInput(1, _bitmapBackground.Get());

			// draw composite on render target bitmap (local bitmap)
			pDeviceContext->SetTransform(::D2D1::Matrix3x2F::Identity());
			pDeviceContext->SetTarget(_renderBitmap.Get());
			pDeviceContext->BeginDraw();
			pDeviceContext->Clear();
			pDeviceContext->DrawImage(_compositeEffect.Get());
			pDeviceContext->EndDraw();

			//restore original device context bitmap and transform
			pDeviceContext->SetTransform(deviceTransform);
			pDeviceContext->SetTarget(oldTarget.Get());

			return displayedItems;
		}
		/**
		*
		*
		*
		*/
		bool SpotLightLayer::DrawNodeSpotlight(::Microsoft::WRL::ComPtr<ID2D1DeviceContext> pDeviceContext, ::NM::ODB::OBJECTUID objectUID)
		{
			// only spotlight if the object is on the currently active layer
			if (_odb->GetValue(objectUID, L"group")->Get<::NM::ODB::OBJECTUID>() != _currentLayerUID)
				return false;


			real_int xpos = _odb->GetValue(objectUID, L"xpos")->Get<::NM::ODB::ODBInt>();
			real_int ypos = _odb->GetValue(objectUID, L"ypos")->Get<::NM::ODB::ODBInt>();
			real_int width = _odb->GetValue(objectUID, L"imagewidth")->Get<::NM::ODB::ODBInt>();
			real_int height = _odb->GetValue(objectUID, L"imageheight")->Get<::NM::ODB::ODBInt>();

			pDeviceContext->FillEllipse(
				::D2D1::Ellipse(
					::D2D1::Point2F(
						static_cast<float>(xpos), 
						static_cast<float>(ypos)), 
					static_cast<float>(width), 
					static_cast<float>(height)), 
				_spotBrush.Get()
				);

			return true;
		}
		/**
		*
		*
		*
		*/
		bool SpotLightLayer::DrawLinkSpotlight(::Microsoft::WRL::ComPtr<ID2D1DeviceContext> pDeviceContext, ::NM::ODB::OBJECTUID objectUID)
		{
			// get connected vertex A
			::NM::ODB::OBJECTUID intfA = _odb->GetValue(objectUID, L"interfaceUID_A")->Get<::NM::ODB::OBJECTUID>();
			assert(intfA != ::NM::ODB::INVALID_OBJECT_UID);
			::NM::ODB::OBJECTUID nodeA = _odb->GetValue(intfA, L"vertexUID")->Get<::NM::ODB::OBJECTUID>();
			assert(nodeA != ::NM::ODB::INVALID_OBJECT_UID);
			// get connected vertex b
			::NM::ODB::OBJECTUID intfB = _odb->GetValue(objectUID, L"interfaceUID_B")->Get<::NM::ODB::OBJECTUID>();
			assert(intfB != ::NM::ODB::INVALID_OBJECT_UID);
			::NM::ODB::OBJECTUID nodeB = _odb->GetValue(intfB, L"vertexUID")->Get<::NM::ODB::OBJECTUID>();
			assert(nodeB != ::NM::ODB::INVALID_OBJECT_UID);
			//////
			//OBJECTUID nodeA = _odb->GetValue(objectUID, L"vertex_a_uid")->Get<::NM::ODB::ODBUID>();
			//OBJECTUID nodeB = _odb->GetValue(objectUID, L"vertex_b_uid")->Get<::NM::ODB::ODBUID>();

			// get link connected nodes group UIDs. 
			OBJECTUID nodeAGroup = _odb->GetValue(nodeA, L"group")->Get<::NM::ODB::OBJECTUID>();
			OBJECTUID nodeBGroup = _odb->GetValue(nodeB, L"group")->Get<::NM::ODB::OBJECTUID>();

			// are either in the active group, if neither in active group, then return false;
			if ((nodeAGroup != _currentLayerUID) && (nodeBGroup != _currentLayerUID))
				return false;

			// if one is not and it is the child that is not, get the child group ID and highlight that as connected node.
			::NM::ODB::OBJECTUID childNode = GetChildNode(nodeA, nodeB);
			if (childNode == nodeA)
			{
				nodeA = nodeAGroup;
			}
			else if (childNode == nodeB)
			{
				nodeB = nodeBGroup;
			}
			// else just leave nodes alone as they are on same level

			real_int xposNodeA = _odb->GetValue(nodeA, L"xpos")->Get<::NM::ODB::ODBInt>();
			real_int yposNodeA = _odb->GetValue(nodeA, L"ypos")->Get<::NM::ODB::ODBInt>();

			real_int xposNodeB = _odb->GetValue(nodeB, L"xpos")->Get<::NM::ODB::ODBInt>();
			real_int yposNodeB = _odb->GetValue(nodeB, L"ypos")->Get<::NM::ODB::ODBInt>();

			real_int controlpointAxpos = _odb->GetValue(objectUID, L"controlpointAxpos")->Get<::NM::ODB::ODBInt>();
			real_int controlpointAypos = _odb->GetValue(objectUID, L"controlpointAypos")->Get<::NM::ODB::ODBInt>();
			real_int controlpointBxpos = _odb->GetValue(objectUID, L"controlpointBxpos")->Get<::NM::ODB::ODBInt>();
			real_int controlpointBypos = _odb->GetValue(objectUID, L"controlpointBypos")->Get<::NM::ODB::ODBInt>();

			real_int linkWidth = _odb->GetValue(objectUID, L"linewidth")->Get<::NM::ODB::ODBInt>();

			// todo check if multiple links and if expanded/collapsed

			// highlight endpoints as this looks better
			DrawNodeSpotlight(pDeviceContext, nodeA);
			DrawNodeSpotlight(pDeviceContext, nodeB);

			// draw linear or biezer line
			if ((controlpointAxpos < 0) ||
				(controlpointAypos < 0) ||
				(controlpointBxpos < 0) ||
				(controlpointBypos < 0))
			{
				pDeviceContext->DrawLine(
					::D2D1::Point2F(static_cast<float>(xposNodeA), static_cast<float>(yposNodeA)),
					::D2D1::Point2F(static_cast<float>(xposNodeB), static_cast<float>(yposNodeB)),
					_spotBrush.Get(),
					static_cast<float>(linkWidth) + 15.0f);
			}
			else
			{
				// draw biezer spotlight
				::Microsoft::WRL::ComPtr<ID2D1PathGeometry> pPathGeometry;
				::Microsoft::WRL::ComPtr<ID2D1GeometrySink> pSink;

				HRESULT hr = _pD2DFactory->CreatePathGeometry(pPathGeometry.GetAddressOf());

				hr = pPathGeometry->Open(pSink.GetAddressOf());

				if (SUCCEEDED(hr))
				{
					pSink->BeginFigure(::D2D1::Point2F(static_cast<float>(xposNodeA), static_cast<float>(yposNodeA)), D2D1_FIGURE_BEGIN_HOLLOW);

					pSink->AddBezier(
						D2D1::BezierSegment(
							::D2D1::Point2F(static_cast<float>(controlpointAxpos), static_cast<float>(controlpointAypos)),
							::D2D1::Point2F(static_cast<float>(controlpointBxpos), static_cast<float>(controlpointBypos)),
							::D2D1::Point2F(static_cast<float>(xposNodeB), static_cast<float>(yposNodeB))));

					pSink->EndFigure(D2D1_FIGURE_END_OPEN);

					pSink->Close();
				}
				pSink.Reset();

				pDeviceContext->DrawGeometry(
					pPathGeometry.Get(),
					_spotBrush.Get(),
					15.0f);			

			}

			return true;
		}
		/**
		*
		*
		*
		*/
		::NM::ODB::OBJECTUID SpotLightLayer::GetChildNode(::NM::ODB::OBJECTUID nodeA, ::NM::ODB::OBJECTUID nodeB)
		{
			// get link connected nodes group UIDs. 
			OBJECTUID nodeAGroup = _odb->GetValue(nodeA, L"group")->Get<::NM::ODB::OBJECTUID>();
			OBJECTUID nodeBGroup = _odb->GetValue(nodeB, L"group")->Get<::NM::ODB::OBJECTUID>();

			// nodes on different levels, find the parent and child.
			::NM::ODB::OBJECTUID rootGroup = ::NM::ODB::INVALID_OBJECT_UID;
			::std::vector<::NM::ODB::OBJECTUID> nodeAGroupList;
			::std::vector<::NM::ODB::OBJECTUID> nodeBGroupList;

			// get hierarcy of nodeA's groups back to root
			::NM::ODB::OBJECTUID currentGroup = nodeAGroup;
			nodeAGroupList.push_back(currentGroup);
			while (currentGroup != rootGroup)
			{
				currentGroup = _odb->GetValue(currentGroup, L"group")->Get<::NM::ODB::ODBUID>();
				nodeAGroupList.push_back(currentGroup);
			}

			// get hierarcy of nodeB's groups back to root
			currentGroup = nodeBGroup;
			nodeBGroupList.push_back(currentGroup);
			while (currentGroup != rootGroup)
			{
				currentGroup = _odb->GetValue(currentGroup, L"group")->Get<::NM::ODB::ODBUID>();
				nodeBGroupList.push_back(currentGroup);
			}


			// if... then NodeB is in the parent group, set the child endpoint (nodeA) to be the group node A
			if (nodeAGroupList.size() > nodeBGroupList.size())
			{
				real_int xposNodeA = _odb->GetValue(nodeAGroup, L"xpos")->Get<::NM::ODB::ODBInt>();
				real_int yposNodeA = _odb->GetValue(nodeAGroup, L"ypos")->Get<::NM::ODB::ODBInt>();
				return nodeA;
			}
			// else if... then NodeA is in the parent group, set the child endpoint (nodeB) to be the group node 
			else if (nodeBGroupList.size() > nodeAGroupList.size())
			{
				real_int xposNodeB = _odb->GetValue(nodeBGroup, L"xpos")->Get<::NM::ODB::ODBInt>();
				real_int yposNodeB = _odb->GetValue(nodeBGroup, L"ypos")->Get<::NM::ODB::ODBInt>();
				return nodeB;
			}


			return ::NM::ODB::INVALID_OBJECT_UID;
		}
		/**
		*
		*
		*
		*/
		void SpotLightLayer::DrawBackground()
		{
			OutputDebugString(L"\nSpotLightLayer::DrawBackground");
			::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget;
			HRESULT hr = GetRenderTarget(pRenderTarget);
			ASSERT(pRenderTarget);

			// get current device context from rendertarget
			::Microsoft::WRL::ComPtr<ID2D1DeviceContext> pDeviceContext = nullptr;
			hr = pRenderTarget.Get()->QueryInterface(IID_ID2D1DeviceContext, &pDeviceContext);
			ASSERT(pDeviceContext);

			// save current device attributes
			D2D1_MATRIX_3X2_F deviceTransform;
			pDeviceContext->GetTransform(&deviceTransform);
			::Microsoft::WRL::ComPtr<ID2D1Image> oldTarget;
			pDeviceContext->GetTarget(&oldTarget);
			D2D1_SIZE_F f = pDeviceContext->GetSize();

			
			::Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> backBrush;
			pDeviceContext->CreateSolidColorBrush(::D2D1::ColorF(::D2D1::ColorF::Black, 0.7f), backBrush.GetAddressOf());

			// set new target as bitmap_background	
			pDeviceContext->SetTarget(_bitmapBackground.Get());
			pDeviceContext->SetTransform(::D2D1::Matrix3x2F::Identity());
			pDeviceContext->BeginDraw();
			pDeviceContext->Clear();
			pDeviceContext->FillRectangle(::D2D1::RectF(0, 0, f.width, f.height), backBrush.Get());
			pDeviceContext->EndDraw();
			pDeviceContext->SetTransform(deviceTransform);
			pDeviceContext->SetTarget(oldTarget.Get());
			
			return;
		}
		/**
		*
		*
		*
		*/
		void SpotLightLayer::CreateBitmaps()
		{
			::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget;
			HRESULT hr = GetRenderTarget(pRenderTarget);
			ASSERT(pRenderTarget);

			// get current device context from rendertarget
			::Microsoft::WRL::ComPtr<ID2D1DeviceContext> pDeviceContext = nullptr;
			hr = pRenderTarget.Get()->QueryInterface(IID_ID2D1DeviceContext, &pDeviceContext);
			ASSERT(pDeviceContext);

			// create new bitmaps
			D2D1_SIZE_F f = pRenderTarget->GetSize();
			float dpiX = 0;
			float dpiY = 0;
			pDeviceContext->GetDpi(&dpiX, &dpiY);

			D2D1_BITMAP_PROPERTIES1 bitmapProperties =
				D2D1::BitmapProperties1(
					D2D1_BITMAP_OPTIONS_TARGET,
					pRenderTarget->GetPixelFormat(),
					dpiX, //g_DPIScaleX,
					dpiY // g_DPIScaleY
					);

			// create the target bitmap
			_renderBitmap.Reset();
			hr = pDeviceContext->CreateBitmap(
				::D2D1::SizeU((UINT32)f.width, (UINT32)f.height),
				nullptr,
				0,
				&bitmapProperties,
				_renderBitmap.GetAddressOf()
				);

			// create the new spotlight bitmap 
			_bitmapSpotlight.Reset();
			hr = pDeviceContext->CreateBitmap(
				::D2D1::SizeU((UINT32)f.width, (UINT32)f.height),
				nullptr,
				0,
				&bitmapProperties,
				_bitmapSpotlight.GetAddressOf()
				);

			// create the new background bitmap 
			_bitmapBackground.Reset();
			hr = pDeviceContext->CreateBitmap(
				::D2D1::SizeU((UINT32)f.width, (UINT32)f.height),
				nullptr,
				0,
				&bitmapProperties,
				_bitmapBackground.GetAddressOf()
				);

			return;
		}
		/**
		*
		*
		*
		*/
		void SpotLightLayer::RefreshContent()
		{
			// clear old bitmaps/brushes (device dependant stuff)
			_spotBrush.Reset();
			_compositeEffect.Reset();
			_renderBitmap.Reset();
			_bitmapBackground.Reset();
			_bitmapSpotlight.Reset();

			// create new bitmaps ready for use
			CreateBitmaps();
			// create the background, only needs recreating on render target change or wm_size event.
			DrawBackground();

			// if we have an object list redraw it.
			if (_objectList.size() > 0)
			{
				::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pTarget;
				GetRenderTarget(pTarget);
				CreateSpotlight();
				OnRender(pTarget, true);
			}

		}


		// ns
	}
}

