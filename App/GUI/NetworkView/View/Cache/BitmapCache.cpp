#include "stdafx.h"
#include "BitmapCache.h"

#pragma comment(lib, "dxguid.lib")
//#include <wincodec.h>
//#include <wrl.h>
//#include <comdef.h>
namespace NM
{
	namespace NetGraph
	{
		/**
		*
		*
		*
		*/
		BitmapCache::BitmapCache():
			_nextIndex(1),
			_pRenderTarget(nullptr)
		{
			// Create WIC factory.
			HRESULT hr = CoCreateInstance(
				CLSID_WICImagingFactory,
				NULL,
				CLSCTX_INPROC_SERVER,
				IID_IWICImagingFactory,
				reinterpret_cast<void **>(&pIWICFactory)
				);
		}
		/**
		*
		*
		*
		*/
		BitmapCache::~BitmapCache()
		{		
			INDEX_ENUM_BITMAP::iterator indexit = mIndexEnum.begin();
			while (indexit != mIndexEnum.end())
			{
				ENUM_BITMAP::iterator stateit = indexit->second.begin();
				while (stateit != indexit->second.end())
				{
					(stateit->second).Reset();// = nullptr;
					++stateit;
				}
				++indexit;
			}
		}
		/**
		*
		*
		*
		*/
		size_t BitmapCache::GetBitmapIndex(::std::wstring FileName, size_t BitmapWidth, size_t BitmapHeight)
		{
			index idx = FindBitmap(FileName, BitmapWidth, BitmapHeight);
			if (idx > 0)
				return  idx;

			HRESULT hr = S_OK;
			// create new images for all ImageStates
			idx = GetNewIndex();
			// save tuple ID to index
			_tupleIndexMap[::std::make_tuple(FileName, BitmapWidth, BitmapHeight)] = idx;

			CreateBitmaps(idx, FileName, BitmapWidth, BitmapHeight);			

			return idx;
		}
		/**
		* CreateBitmaps 
		*
		* Creates all combinations of bitmaps
		*/
		HRESULT BitmapCache::CreateBitmaps(index idx, ::std::wstring FileName, size_t BitmapWidth, size_t BitmapHeight)
		{
			HRESULT hr = S_OK;

			// temp remove any applied transform to the rendertarget/devicecontext
			D2D1::Matrix3x2F originalTransform;
			_pRenderTarget->GetTransform(&originalTransform);
			_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
			
#pragma region StateOverlayImages


			// Graph State - Disabled Overlay
			//////////////////////////////////////////////////////////////////////////////////////
			::Microsoft::WRL::ComPtr<ID2D1Bitmap> pGraphStateDisabled = nullptr;
			hr = LoadBitmapFromFile(L"disabled.png", pGraphStateDisabled, 15, 15);
			
			// Selected State - Selected Overlay
			//////////////////////////////////////////////////////////////////////////////////////
			::Microsoft::WRL::ComPtr<ID2D1Bitmap> pSelectedStateSelected = nullptr;
			hr = LoadBitmapFromFile(L"greentick.png", pSelectedStateSelected, 15, 15);

			// Operational State - State Down
			//////////////////////////////////////////////////////////////////////////////////////
			::Microsoft::WRL::ComPtr<ID2D1Bitmap> pOperationalStateDown= nullptr;
			hr = LoadBitmapFromFile(L"DownArrow.png", pOperationalStateDown, 15, 15);


			// Operational State - State Unknown
			//////////////////////////////////////////////////////////////////////////////////////
			::Microsoft::WRL::ComPtr<ID2D1Bitmap> pOperationalStateUnknown = nullptr;
			hr = LoadBitmapFromFile(L"unknown.png", pOperationalStateUnknown, 15, 15);
			
			

#pragma endregion StateOverlayImages


#pragma region OperationalImages
			// Create Operational Images.

			//////////////////////////////////////////////////////////////////////////////////////
			// 1. load normal operational state image
			//////////////////////////////////////////////////////////////////////////////////////
			::Microsoft::WRL::ComPtr<ID2D1Bitmap> pNormalOperationalBitmap = nullptr;
			hr = LoadBitmapFromFile(FileName, pNormalOperationalBitmap);


			

#pragma endregion OperationalImages


#pragma region GraphStateImages
		
			// NodeOperationalState::	Up
			// NodeAdminState::		Disabled 
			// NodeSelectedState::		Unselected
			//////////////////////////////////////////////////////////////////////////////////////
			::Microsoft::WRL::ComPtr<ID2D1Bitmap1> pDisabledUnselectedBitmap;
			if (SUCCEEDED(hr))
			{
				// convert to grayscale
				::Microsoft::WRL::ComPtr<ID2D1Bitmap1> pGreyScaleBitmap2;
				hr = CreateGrayScaleBitmap(pNormalOperationalBitmap, pGreyScaleBitmap2);
				if (SUCCEEDED(hr))
				{
					::Microsoft::WRL::ComPtr<ID2D1Bitmap> srcbmp1 = pGreyScaleBitmap2;
					hr = CreateCompositeBitmap(srcbmp1, pGraphStateDisabled, pDisabledUnselectedBitmap, ImageStateOverlayPosition::Center);  // topleft
					srcbmp1 = nullptr;
				}
				pGreyScaleBitmap2.Reset();
			}
			

			// NodeOperationalState::	Up
			// NodeAdminState::		Disabled 
			// NodeSelectedState::		Selected
			//////////////////////////////////////////////////////////////////////////////////////
			::Microsoft::WRL::ComPtr<ID2D1Bitmap1> pDisabledSelectedBitmap;
			::Microsoft::WRL::ComPtr<ID2D1Bitmap1> pGreyScaleDisabledBitmap;

			if (SUCCEEDED(hr))
			{
				// convert to grayscale
				::Microsoft::WRL::ComPtr<ID2D1Bitmap1> pGreyScaleBitmap2;
				hr = CreateGrayScaleBitmap(pNormalOperationalBitmap, pGreyScaleBitmap2);
				// add red cross for graph disabled
				if (SUCCEEDED(hr))
				{
					::Microsoft::WRL::ComPtr<ID2D1Bitmap> srcbmp1 = pGreyScaleBitmap2;
					hr = CreateCompositeBitmap(srcbmp1, pGraphStateDisabled, pGreyScaleDisabledBitmap, ImageStateOverlayPosition::Center); // topleft
					srcbmp1 = nullptr;
				}
				// add green tick for selected
				if (SUCCEEDED(hr))
				{
					::Microsoft::WRL::ComPtr<ID2D1Bitmap> srcbmp1 = pGreyScaleDisabledBitmap;
					hr = CreateCompositeBitmap(srcbmp1, pSelectedStateSelected, pDisabledSelectedBitmap, ImageStateOverlayPosition::TopRight);
					srcbmp1 = nullptr;
				}
				pGreyScaleBitmap2.Reset();
			}


			// NodeOperationalState::	Up
			// NodeAdminState::		Enabled 
			// NodeSelectedState::		Unselected
			//////////////////////////////////////////////////////////////////////////////////////
			/*
			* Just normal UP operational image
			*/


			// NodeOperationalState::	Up
			// NodeAdminState::		Enabled 
			// NodeSelectedState::		Selected
			//////////////////////////////////////////////////////////////////////////////////////
			::Microsoft::WRL::ComPtr<ID2D1Bitmap1> pEnabledSelectedBitmap;
			if (SUCCEEDED(hr))			{
				
				// add green tick for selected
				if (SUCCEEDED(hr))
				{
					hr = CreateCompositeBitmap(pNormalOperationalBitmap, pSelectedStateSelected, pEnabledSelectedBitmap, ImageStateOverlayPosition::TopRight);
				}
			}
			


#pragma endregion GraphStateImages

			// set transform back to original
			_pRenderTarget->SetTransform(&originalTransform);

#pragma region CacheBitmaps

			// NodeOperationalState::	Up
			// NodeAdminState::		Enabled 
			// NodeSelectedState::		UnSelected
			CacheBitmap(
				idx, 
				::std::make_tuple(
					NodeOperationalState::Up, 
					NodeAdminState::Enabled, 
					NodeSelectedState::Unselected
					),
				pNormalOperationalBitmap);

			// NodeOperationalState::	Up
			// NodeAdminState::		Enabled 
			// NodeSelectedState::		Selected
			::Microsoft::WRL::ComPtr<ID2D1Bitmap> pTemp = pEnabledSelectedBitmap;
			CacheBitmap(
				idx,
				::std::make_tuple(
					NodeOperationalState::Up,
					NodeAdminState::Enabled,
					NodeSelectedState::Selected
					),
				pTemp);
			pTemp = nullptr;


			// NodeOperationalState::	Up
			// NodeAdminState::		Disabled 
			// NodeSelectedState::		UnSelected
			pTemp = pDisabledUnselectedBitmap;
			CacheBitmap(
				idx,
				::std::make_tuple(
					NodeOperationalState::Up,
					NodeAdminState::Disabled,
					NodeSelectedState::Unselected
					),
				pTemp);
			pTemp = nullptr;


			// NodeOperationalState::	Up
			// NodeAdminState::		Disabled 
			// NodeSelectedState::		Selected
			pTemp = pDisabledSelectedBitmap;
			CacheBitmap(
				idx,
				::std::make_tuple(
					NodeOperationalState::Up,
					NodeAdminState::Disabled,
					NodeSelectedState::Selected
					),
				pTemp);
			pTemp = nullptr;


			// NodeOperationalState::	Down
			// NodeAdminState::		Disabled 
			// NodeSelectedState::		Unselected


			// NodeOperationalState::	Down
			// NodeAdminState::		Disabled 
			// NodeSelectedState::		Selected


			// NodeOperationalState::	Unknown
			// NodeAdminState::		Disabled 
			// NodeSelectedState::		Unselected


			// NodeOperationalState::	Unknown
			// NodeAdminState::		Disabled 
			// NodeSelectedState::		Selected

			// complete default return in GetBitmap for bad params when the above is done. Unknown/Disabled/ (Un/Selected)



#pragma endregion CacheBitmaps

			// release local resource
			pGreyScaleDisabledBitmap.Reset();

			pNormalOperationalBitmap.Reset();

			pGraphStateDisabled.Reset();
			pSelectedStateSelected.Reset();
			pOperationalStateDown.Reset();
			pOperationalStateUnknown.Reset();
			
			pDisabledUnselectedBitmap.Reset();
			pDisabledSelectedBitmap.Reset();
			pEnabledSelectedBitmap.Reset();

			return hr;
		}
		/**
		*
		*
		*
		*/
		size_t BitmapCache::CacheBitmap(index idx, STATEMASK stateMask, ::Microsoft::WRL::ComPtr<ID2D1Bitmap> ppBitmap)
		{
			ASSERT(ppBitmap);

			// check we have an index to ENUM_BITMAP map, if not create one with an emprty ENUM_BITMAP
			INDEX_ENUM_BITMAP::iterator indexit = mIndexEnum.find(idx);
			if (indexit == mIndexEnum.end())
			{
				// create new entry for this index
				// first create inner ENUM_BITMAP(STATEMASK) to be value in INDEX_ENUM_BITMAP
				ENUM_BITMAP tmp;
				//tmp [stateMask]= nullptr;

				// insert this empty state map into the INDEX_ENUM_BITMAP map
				std::pair<INDEX_ENUM_BITMAP::iterator, bool> retval = mIndexEnum.insert(::std::make_pair(idx, tmp));
				indexit = ::std::get<0>(retval);
			}


			// Check to see if we have a STATEMASK->Bitmap already in the ENUM_BITMAP map for this index, if not create one

			// INDEX_ENUM_BITMAP::Second(ENUM_BITMAP) // ENUM_BITMAP->Find...
			ENUM_BITMAP::iterator enumit = indexit->second.find(stateMask);
			if (enumit == indexit->second.end())
			{
				
				// insert this empty state map into the INDEX_ENUM_BITMAP map
				std::pair<ENUM_BITMAP::iterator, bool> retval = indexit->second.insert(::std::make_pair(stateMask, nullptr));
				enumit = ::std::get<0>(retval);

			}
			//else
			//{
			//	OutputDebugString(L"\n Replacing Existing Bitmap in Cache");
			//}

			// save reference to the passed bitmap for this index/enum pair
			(enumit->second).Reset(); // = nullptr;
			enumit->second = ppBitmap;

			return idx;
		}
		/**
		*
		*
		*
		*/
		BitmapCache::index BitmapCache::FindBitmap(filename fn, width w, height h)
		{
			TUPLE_INDEX_MAP::iterator it = _tupleIndexMap.find(::std::make_tuple(fn, w, h));
			if (it == _tupleIndexMap.end())
				return 0;

			return it->second;
		}
		/**
		*
		*
		*
		*/
		BitmapCache::index BitmapCache::GetNewIndex()
		{
			return _nextIndex++;
		}
		/**
		*
		*
		*
		*/
		ComBitmap BitmapCache::GetBitmap(size_t BitmapIndex, NodeOperationalState OpState, NodeAdminState GraphState, NodeSelectedState SelectState)
		{
			STATEMASK stateMask = ::std::make_tuple(OpState, GraphState, SelectState);

			INDEX_ENUM_BITMAP::iterator indexit = mIndexEnum.find(BitmapIndex);
			if (indexit == mIndexEnum.end())
			{
				ASSERT(false);
				return nullptr; // bad index
				// TODO return 
				// NodeOperationalState::	Unknown
				// NodeAdminState::		Disabled 
				// NodeSelectedState		SelectState param
			}

			ENUM_BITMAP::iterator enumit = indexit->second.find(stateMask);
			if (enumit == indexit->second.end())
			{
				// should always have every state even if bitmap points to nullptr
				throw ::std::runtime_error("BitmapCache indexes corrupt");
			}

			// return requested bitmap
			return enumit->second;
		}
		/**
		* RefreshContent
		*
		* Called when the devicecontext changes and therefore device dependant resources need to
		* be rebuilt/redrawn.
		*/
		void BitmapCache::RefreshContent(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget)
		{
			HRESULT hr = S_OK;

			// save new render target
			if (pRenderTarget == nullptr)
			{
				_pRenderTarget.Reset();
				return;
			}
			_pRenderTarget = pRenderTarget;

			// clear old bitmaps, leave data structure in place as we will update the bitmaps pointers
			INDEX_ENUM_BITMAP::iterator indexit = mIndexEnum.begin();
			while (indexit != mIndexEnum.end())
			{
				ENUM_BITMAP::iterator enumit = indexit->second.begin();
				while (enumit != indexit->second.end())
				{
					(enumit->second).Reset(); // = nullptr;
					++enumit;
				}
				++indexit;
			}

			// create new bitmaps
			TUPLE_INDEX_MAP::iterator tupleit = _tupleIndexMap.begin();
			while (tupleit != _tupleIndexMap.end())
			{
				CreateBitmaps(
					tupleit->second,					// exsiting index id
					::std::get<0>(tupleit->first),		// filename
					::std::get<1>(tupleit->first),		// width override
					::std::get<2>(tupleit->first));		// height override

				++tupleit;
			}
		
			_dragOverlayCursor.Reset();
			hr = LoadBitmapFromFile(L"dragoverlay.png", _dragOverlayCursor, 32, 32);
			if (FAILED(hr))
				throw ::std::runtime_error("Failed to load drag overlay cursor");
			
			return;
		}
		/**
		*
		*
		*
		*/
		HRESULT BitmapCache::CreateCompositeBitmap(::Microsoft::WRL::ComPtr<ID2D1Bitmap> ppSrcBitmap1, ::Microsoft::WRL::ComPtr<ID2D1Bitmap> ppSrcBitmap2, ::Microsoft::WRL::ComPtr<ID2D1Bitmap1> &ppDstBitmap, ImageStateOverlayPosition overlayPosition)
		{
			HRESULT hr = S_OK;
			
			::Microsoft::WRL::ComPtr<ID2D1DeviceContext> pDeviceContext = nullptr;
			hr = _pRenderTarget.Get()->QueryInterface(IID_ID2D1DeviceContext, &pDeviceContext);

			// get src bitmap size
			D2D_SIZE_F srcSize = ppSrcBitmap1->GetSize();
			// create properties for new bitmap taking pixel size from src
			D2D1_BITMAP_PROPERTIES1 bitmapProperties =
				D2D1::BitmapProperties1(
					D2D1_BITMAP_OPTIONS_TARGET,
					ppSrcBitmap1->GetPixelFormat(),
					0, //g_DPIScaleX,
					0 // g_DPIScaleY
					);

			// create the new bitmap (empty)
			hr = pDeviceContext->CreateBitmap(
				::D2D1::SizeU((UINT32)srcSize.width, (UINT32)srcSize.height),
				nullptr,
				0,
				&bitmapProperties,
				ppDstBitmap.GetAddressOf()
				);

			D2D_SIZE_F srcSize2 = ppSrcBitmap2->GetSize();
			D2D1_RECT_F overlayRect = { 0 };
			switch (overlayPosition)
			{
			case ImageStateOverlayPosition::TopLeft:
				overlayRect = ::D2D1::RectF(
					0,									// left
					0,									// top
					srcSize2.width,						// right
					srcSize2.height						// bottom
					);
				break;

			case ImageStateOverlayPosition::TopRight:
				overlayRect = ::D2D1::RectF(
					srcSize.width - srcSize2.width,		// left
					0,									// top
					srcSize.width,						// right
					srcSize2.height						// bottom
					);
				break;

			case ImageStateOverlayPosition::BottomLeft:
				overlayRect = ::D2D1::RectF(
					0,									// left
					srcSize.height - srcSize2.height,	// top
					srcSize2.width,						// right
					srcSize.height						// bottom
					);
				break;

			case ImageStateOverlayPosition::BottomRight:
				overlayRect = ::D2D1::RectF(
					srcSize.width - srcSize2.width,		// left
					srcSize.height - srcSize2.height,	// top
					srcSize.width,						// right
					srcSize.height						// bottom
					);
				break;

			case ImageStateOverlayPosition::Center:
				overlayRect = ::D2D1::RectF(
					(srcSize.width/2) - (srcSize2.width/2),		// left
					(srcSize.height/2) - (srcSize2.height/2),	// top
					(srcSize.width / 2) + (srcSize2.width / 2),						// right
					(srcSize.height / 2) + (srcSize2.height / 2)						// bottom
					);
				break;

			default:
				overlayRect = ::D2D1::RectF(
					(srcSize.width / 2) - (srcSize2.width / 2),		// left
					(srcSize.height / 2) - (srcSize2.height / 2),	// top
					(srcSize.width / 2) + (srcSize2.width / 2),						// right
					(srcSize.height / 2) + (srcSize2.height / 2)						// bottom
					);
				break;

			}

			// save current rendertarget
			::Microsoft::WRL::ComPtr<ID2D1Image> oldTarget;
			pDeviceContext->GetTarget(&oldTarget);
			// set new target as new bitmap
			pDeviceContext->SetTarget(ppDstBitmap.Get());
			// draw on new bitmap
			pDeviceContext->BeginDraw();
			pDeviceContext->Clear();
			pDeviceContext->DrawBitmap(ppSrcBitmap1.Get());
			pDeviceContext->DrawBitmap(ppSrcBitmap2.Get(), overlayRect);
			pDeviceContext->EndDraw();
			//restore original render target
			pDeviceContext->SetTarget(oldTarget.Get());
			// release temporarys
			oldTarget.Reset();
			
			return hr;
		}

		/**
		* CreateGrayScaleBitmap
		*
		* create a gray scale bitmap
		*/
		HRESULT BitmapCache::CreateGrayScaleBitmap(::Microsoft::WRL::ComPtr<ID2D1Bitmap> ppSrcBitmap, ::Microsoft::WRL::ComPtr<ID2D1Bitmap1> &ppDstBitmap)
		{
			HRESULT hr = S_OK;

			::Microsoft::WRL::ComPtr<ID2D1DeviceContext> pDeviceContext = nullptr;
			hr = _pRenderTarget.Get()->QueryInterface(IID_ID2D1DeviceContext, &pDeviceContext);
	
			// get src bitmap size
			D2D_SIZE_F srcSize = ppSrcBitmap->GetSize();
			// create properties for new bitmap taking pixel size from src
			D2D1_BITMAP_PROPERTIES1 bitmapProperties =
				D2D1::BitmapProperties1(
					D2D1_BITMAP_OPTIONS_TARGET,
					ppSrcBitmap->GetPixelFormat(),
					0, //g_DPIScaleX,
					0 // g_DPIScaleY
					);

			// create the new grayscale bitmap (empty)
			hr = pDeviceContext->CreateBitmap(
				::D2D1::SizeU((UINT32)srcSize.width, (UINT32)srcSize.height),
				nullptr,
				0,
				&bitmapProperties,
				ppDstBitmap.GetAddressOf()
				);

			// convert to gray
			::Microsoft::WRL::ComPtr<ID2D1Effect> colorMatrixEffect;
			hr = pDeviceContext->CreateEffect(CLSID_D2D1Grayscale, &colorMatrixEffect);
			colorMatrixEffect->SetInput(0, ppSrcBitmap.Get());

			// save current rendertarget
			::Microsoft::WRL::ComPtr<ID2D1Image> oldTarget;
			pDeviceContext->GetTarget(&oldTarget);
			// set new target as new bitmap
			pDeviceContext->SetTarget(ppDstBitmap.Get());
			// draw on new bitmap
			pDeviceContext->BeginDraw();
			pDeviceContext->Clear();
			pDeviceContext->DrawImage(colorMatrixEffect.Get());
			pDeviceContext->EndDraw();

			//restore original render target
			pDeviceContext->SetTarget(oldTarget.Get());

			// release temporarys
			oldTarget.Reset();
			//pBitmapGray->Release();
			colorMatrixEffect.Reset();	

			return hr;
		}
		/**
		*
		*
		*
		*/
		HRESULT BitmapCache::LoadBitmapFromFile(
			filename fileName,
			::Microsoft::WRL::ComPtr<ID2D1Bitmap> &ppBitmap,
			UINT destinationWidth,
			UINT destinationHeight
			)
		{
			IWICBitmapDecoder *pDecoder = NULL;
			IWICBitmapFrameDecode *pSource = NULL;
			IWICStream *pStream = NULL;
			IWICFormatConverter *pConverter = NULL;
			IWICBitmapScaler *pScaler = NULL;
			IWICPalette *pPalette = NULL;
			HRESULT hr = S_OK;

			if (!_pRenderTarget) throw ::std::runtime_error("LoadBitmapFromFile : Render Target is NULL.");

			if (SUCCEEDED(hr))
			{
				// Create a WIC stream to map onto the memory.
				hr = pIWICFactory->CreateStream(&pStream);
			}
			if (SUCCEEDED(hr))
			{
				::std::wstring file = L"C:\\Users\\Simon\\Documents\\Personal\\Source Code\\Projects\\NetworkModelv2\\Exe\\res\\" + fileName;
				hr = pStream->InitializeFromFilename(file.c_str(), GENERIC_READ);
			}
			if (SUCCEEDED(hr))
			{
				// Create a decoder for the stream.
				hr = pIWICFactory->CreateDecoderFromStream(
					pStream,
					NULL,
					WICDecodeMetadataCacheOnLoad,
					&pDecoder
					);
			}
			if (SUCCEEDED(hr))
			{
				// Create the initial frame.
				hr = pDecoder->GetFrame(0, &pSource);
			}
			if (SUCCEEDED(hr))
			{
				hr = pIWICFactory->CreatePalette(&pPalette);
			}
			//if (SUCCEEDED(hr))
			//{
				//hr = pSource->CopyPalette(pPalette);
				//BOOL bAlpha;
				//hr = pPalette->HasAlpha(&bAlpha);
				//OutputDebugString(((bAlpha == TRUE) ? L"\nHas Alpha" : L"\nDOES NOT HAVE Alpha"));
				//hr = S_OK;
			//}

			if (SUCCEEDED(hr))
			{
				// Convert the image format to 32bppPBGRA
				// (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
				hr = pIWICFactory->CreateFormatConverter(&pConverter);
			}
			if (SUCCEEDED(hr))
			{
				// If a new width or height was specified, create an
				// IWICBitmapScaler and use it to resize the image.
				if (destinationWidth != 0 || destinationHeight != 0)
				{
					UINT originalWidth, originalHeight;
					hr = pSource->GetSize(&originalWidth, &originalHeight);
					if (SUCCEEDED(hr))
					{
						if (destinationWidth == 0)
						{
							FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(originalHeight);
							destinationWidth = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
						}
						else if (destinationHeight == 0)
						{
							FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(originalWidth);
							destinationHeight = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
						}

						hr = pIWICFactory->CreateBitmapScaler(&pScaler);
						if (SUCCEEDED(hr))
						{
							hr = pScaler->Initialize(
								pSource,
								destinationWidth,
								destinationHeight,
								WICBitmapInterpolationModeCubic
								);
							if (SUCCEEDED(hr))
							{
								hr = pConverter->Initialize(
									pScaler,
									GUID_WICPixelFormat32bppPBGRA,					// Direct2D requires bitmap sources to be in the a 32bppPBGRA format for rendering
									WICBitmapDitherTypeNone,
									NULL,
									0.0f,
									WICBitmapPaletteTypeMedianCut
									);
							}
						}
					}
				}
				else
				{

					hr = pConverter->Initialize(
						pSource,
						GUID_WICPixelFormat32bppPBGRA,								// Direct2D requires bitmap sources to be in the a 32bppPBGRA format for rendering
						WICBitmapDitherTypeNone,
						NULL,
						0.f,
						WICBitmapPaletteTypeMedianCut
						);
				}
			}
			if (SUCCEEDED(hr))
			{
				//create a Direct2D bitmap from the WIC bitmap.
				hr = _pRenderTarget->CreateBitmapFromWicBitmap(
					pConverter,
					NULL,
					ppBitmap.GetAddressOf()
					);
			}

			SafeRelease(&pStream);
			SafeRelease(&pDecoder);
			SafeRelease(&pSource);
			SafeRelease(&pConverter);
			SafeRelease(&pScaler);
			return hr;
		}
		// ns
	}
}
