#pragma once
#include <map>
#include <tuple>
#include <string>
#include <wrl.h>
#include <vector>
#include <d2d1.h>
#include <d2d1_1.h>
#include <D2d1_1helper.h>
#include <d2d1effects.h>
#include <d2d1effecthelpers.h>
#include <d2d1effects_2.h>

namespace NM
{
	namespace NetGraph
	{
		enum class NodeOperationalState{ Up = 1, Down = 2, Unknown = 3};		// network operational state
		enum class NodeAdminState { Enabled = 1, Disabled = 2};					// enabled for use in graph algos 
		enum class NodeSelectedState { Selected=1, Unselected=2 };					// selected object yes/no
		enum class ImageStateOverlayPosition {TopLeft, TopRight, BottomLeft, BottomRight, Center};

		typedef ::Microsoft::WRL::ComPtr<ID2D1Bitmap> ComBitmap;

		class BitmapCache
		{
		public:
			BitmapCache();
			~BitmapCache();

			virtual void RefreshContent(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget) final;
			size_t GetBitmapIndex(::std::wstring FileName, size_t BitmapWidth, size_t BitmapHeight);
			ComBitmap GetBitmap(size_t BitmapIndex, NodeOperationalState, NodeAdminState, NodeSelectedState);
			ComBitmap GetDragOverlayBitmap() { return _dragOverlayCursor; }

		private:
			typedef size_t index;
			typedef ::std::wstring filename;
			typedef size_t width;
			typedef size_t height;
	
			typedef ::std::map<::std::tuple<filename, width, height>, index> TUPLE_INDEX_MAP;
			TUPLE_INDEX_MAP _tupleIndexMap;
			typedef ::std::tuple<NodeOperationalState, NodeAdminState, NodeSelectedState> STATEMASK;
			typedef ::std::map<STATEMASK, ComBitmap> ENUM_BITMAP;
			typedef ::std::map<index, ENUM_BITMAP> INDEX_ENUM_BITMAP;
			INDEX_ENUM_BITMAP	mIndexEnum;		

			IWICImagingFactory *pIWICFactory;
			index _nextIndex;
			index FindBitmap(filename, width, height);
			index GetNewIndex();
			HRESULT CreateBitmaps(index, ::std::wstring FileName, size_t BitmapWidth, size_t BitmapHeight);
			index CacheBitmap(index, STATEMASK, ::Microsoft::WRL::ComPtr<ID2D1Bitmap>);
			HRESULT CreateGrayScaleBitmap(::Microsoft::WRL::ComPtr<ID2D1Bitmap> ppSrcBitmap, ::Microsoft::WRL::ComPtr<ID2D1Bitmap1> &ppDstBitmap);
			HRESULT CreateCompositeBitmap(::Microsoft::WRL::ComPtr<ID2D1Bitmap> ppSrcBitmap1, ::Microsoft::WRL::ComPtr<ID2D1Bitmap> ppSrcBitmap2, ::Microsoft::WRL::ComPtr<ID2D1Bitmap1> &ppDstBitmap, ImageStateOverlayPosition);

			::Microsoft::WRL::ComPtr<ID2D1Bitmap>	_dragOverlayCursor;			// overlay used to create drag bitmap

			::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> _pRenderTarget;

			HRESULT LoadBitmapFromFile(
				filename fileName,
				::Microsoft::WRL::ComPtr<ID2D1Bitmap> &ppBitmap,
				UINT destinationWidth = 0,
				UINT destinationHeight = 0
				);

			template<class Interface>
			inline void SafeRelease( Interface **ppInterfaceToRelease)
			{
				if (*ppInterfaceToRelease != nullptr)
				{
					(*ppInterfaceToRelease)->Release();

					//(*ppInterfaceToRelease) = nullptr;
				}
			}

		};


	}
}

