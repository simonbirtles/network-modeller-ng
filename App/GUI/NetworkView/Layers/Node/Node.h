#pragma once
#include "Interfaces\ObjectDatabaseDefines.h"

#include "BitmapCache.h"
#include <wrl.h>
#include <set>

struct IDWriteTextFormat;
struct IDWriteTextLayout;
struct ID2D1SolidColorBrush;

namespace NM
{
	namespace NetGraph
	{
		class BitmapCache;
		

		class Node
		{
		public:
			Node(
				::NM::ODB::OBJECTUID objectUID,
				::NM::ODB::OBJECTUID groupUID,
				long xpos, 
				long ypos, 
				size_t width, 
				size_t height, 
				::std::wstring displayname, 
				::std::wstring bitmapname, 
				IDWriteFactory* writeFactory, 
				::std::shared_ptr<BitmapCache> pBitmapCache
				);
			~Node();
			typedef ::std::set<::std::pair<int, int>> NODEPOINTS;

			Node(const Node &rhs);				// copy ctor
			Node& operator=(const Node &rhs);			// assignment operator
			
			void Draw(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget>&, ::NM::ODB::OBJECTUID& groupUID);
			bool IsPointInBitmap(POINT &pt);
			void RefreshContent(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget>);			// e.g. onsize 
			void UpdateValues(::NM::ODB::ATTRIBUTENAME&, ::NM::ODB::Value&);		// e.g. on DatabaseUpdate

			::NM::ODB::OBJECTUID GetNodeUID() { return _objectUID; };
			void SetPos(long xpos, long ypos);
			void GetPos(long &xpos, long &ypos);
			void GetSize(size_t &BitmapWidth, size_t &BitmapHeight) { BitmapWidth = _bitmapWidth; BitmapHeight = _bitmapHeight; };
			D2D1_RECT_F GetRect();
			void SetOpacity(float Opacity = 1.0f);
			void SetDragImageStatus(bool bSetDrag);
			bool GetDragImageStatus() { return _dragImageRequired; };
			void SetOperationalState(NodeOperationalState);
			NodeOperationalState GetOperationalState() { return _nodeOperationalState; };
			void SetAdminState(NodeAdminState);
			NodeAdminState GetAdminState() { return _nodeAdminState; };
			void SetSelectedState(NodeSelectedState);
			NodeSelectedState GetSelectedState() { return  _nodeSelectedState; };
			::NM::ODB::OBJECTUID GetGroup() {return _groupUID;};
			void GetNodePoints(int spatialGridSize, NODEPOINTS&);


		private:

			NodeOperationalState	_nodeOperationalState;
			NodeAdminState			_nodeAdminState;
			NodeSelectedState		_nodeSelectedState;

			// metadata
			::NM::ODB::OBJECTUID _objectUID;
			::NM::ODB::OBJECTUID _groupUID;
			long _xpos, _ypos;
			long _bitmapWidth, _bitmapHeight;
			::std::wstring _strDisplayName;
			::std::wstring _strBitmapName;
			float _opacity;
			bool _dragImageRequired;
			float _textStringWidth;
			float _textStringHeight;
			

			// internal data
			D2D1_POINT_2F _origin;			//  upper-left point of text
			::std::shared_ptr<BitmapCache>	_pBitmapCache;
			
			
			size_t _bitmapCacheNormalIndex;

			IDWriteFactory*			_pDWriteFactory;
			::Microsoft::WRL::ComPtr<IDWriteTextFormat>		_pTextFormat;
			::Microsoft::WRL::ComPtr<IDWriteTextLayout>		_pTextLayout;
			::Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>	_pTextWhiteBrush;

			HRESULT UpdateNodeText();
			void UpdateOrigin();

			template<class Interface>
			inline void SafeRelease(Interface **ppInterfaceToRelease)
			{
				if (*ppInterfaceToRelease != nullptr)
				{
					(*ppInterfaceToRelease)->Release();
					(*ppInterfaceToRelease) = nullptr;
				}
			}

		};

// ns
	}
}
