#pragma once
#include "BaseLayer.h"
#include "Interfaces\ObjectDatabaseDefines.h"
#include <wrl.h>
#include <vector>

namespace NM
{
	namespace ODB
	{
		class IObjectDatabase;
	}

	namespace NetGraph
	{

		class SpotLightLayer : public BaseLayer
		{
		public:
			SpotLightLayer();
			~SpotLightLayer();

			HRESULT OnRender(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> &pRenderTarget, bool bRedraw);
			LRESULT ProcessMessage(UINT, WPARAM, LPARAM);
			bool ShouldRender() { return (_objectList.size() > 0); };
			bool Initialise();
			::std::wstring GetLayerText() { return L"Spotlight Layer"; }

			void SetSpotlightOn(::std::vector<::NM::ODB::OBJECTUID>&);
			void SetSpotlightOff();

		private:
			IDWriteFactory* pDWriteFactory_;
			::Microsoft::WRL::ComPtr<IDWriteTextFormat> pTextFormat_;
			//::Microsoft::WRL::ComPtr<IDWriteTextFormat> pTextNodeFormat_;
			const wchar_t* wszText_;
			UINT32 cTextLength_;
			::Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pTextWhiteBrush_;

			bool											_spotlightOn;
			::std::unique_ptr<::NM::ODB::IObjectDatabase>	_odb;
			::std::vector<::NM::ODB::OBJECTUID>				_objectList;
			::NM::ODB::OBJECTUID	_currentLayerUID;


			::Microsoft::WRL::ComPtr<ID2D1Factory>			_pD2DFactory;
			::Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>	_spotBrush;
			::Microsoft::WRL::ComPtr<ID2D1Effect>			_compositeEffect;
			::Microsoft::WRL::ComPtr<ID2D1Bitmap1>			_renderBitmap;
			::Microsoft::WRL::ComPtr<ID2D1Bitmap1>			_bitmapBackground;
			::Microsoft::WRL::ComPtr<ID2D1Bitmap1>			_bitmapSpotlight;

			bool DrawNodeSpotlight(::Microsoft::WRL::ComPtr<ID2D1DeviceContext>, ::NM::ODB::OBJECTUID);
			bool DrawLinkSpotlight(::Microsoft::WRL::ComPtr<ID2D1DeviceContext>, ::NM::ODB::OBJECTUID);
			void DrawBackground();
			void CreateBitmaps();
			size_t CreateSpotlight();
			void RefreshContent();
			::NM::ODB::OBJECTUID GetChildNode(::NM::ODB::OBJECTUID, ::NM::ODB::OBJECTUID);

		};

		// ns
	}
}

