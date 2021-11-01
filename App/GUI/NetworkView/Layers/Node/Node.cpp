#include "stdafx.h"
#include "Node.h"

#include <d2d1.h>
#include <D2d1_1helper.h>
#include <d2d1effects.h>
#include <d2d1effecthelpers.h>
#include <dwrite_3.h>
#include <string>

namespace NM
{
	namespace NetGraph
	{
		/**
		*
		*
		*
		*/
		Node::Node(
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
			) :
			_objectUID(objectUID),
			_groupUID(groupUID),
			_xpos(xpos),
			_ypos(ypos),
			_bitmapWidth(width),
			_bitmapHeight(height),
			_opacity(1.0f),
			_strDisplayName(displayname),
			_strBitmapName(bitmapname),
			_pDWriteFactory(writeFactory),
			_pBitmapCache(pBitmapCache),
			_pTextFormat(nullptr),
			_pTextLayout(nullptr),
			_pTextWhiteBrush(nullptr),
			_bitmapCacheNormalIndex(0),
			_nodeOperationalState(NodeOperationalState::Up),
			_nodeAdminState(NodeAdminState::Enabled),
			_nodeSelectedState(NodeSelectedState::Unselected),
			_dragImageRequired(false),
			_textStringWidth(0.0f),
			_textStringHeight(0.0f)
		{
		}
		/**
		*
		*
		*
		*/
		Node::~Node()
		{
			_pTextFormat.Reset();
			_pTextLayout.Reset();
			_pTextWhiteBrush.Reset();
			_pDWriteFactory = nullptr;
			_pBitmapCache = nullptr;
		}
		/**
		*
		*
		* copy ctor
		*/
		Node::Node(const Node &rhs)				
		{
			_objectUID = rhs._objectUID;
			_groupUID = rhs._groupUID;
			_nodeOperationalState = rhs._nodeOperationalState;
			_nodeAdminState = rhs._nodeAdminState;
			_nodeSelectedState = rhs._nodeSelectedState;
			_xpos = rhs._xpos;
			_ypos = rhs._ypos;
			_bitmapWidth = rhs._bitmapWidth;
			_bitmapHeight = rhs._bitmapHeight;
			_strDisplayName = rhs._strDisplayName;
			_strBitmapName = rhs._strBitmapName;
			_pBitmapCache = rhs._pBitmapCache;
			_opacity = rhs._opacity;
			_origin = rhs._origin;
			_bitmapCacheNormalIndex = rhs._bitmapCacheNormalIndex;
			_pDWriteFactory = rhs._pDWriteFactory;
			_pTextFormat = nullptr;
			_pTextLayout = nullptr;
			_pTextWhiteBrush = nullptr;
			_dragImageRequired = rhs._dragImageRequired;
		}
		/**
		*
		*
		*
		*/
		Node& Node::operator=(const Node &rhs)			// assignment operator
		{
			_objectUID = rhs._objectUID;
			_groupUID = rhs._groupUID;
			_nodeOperationalState = rhs._nodeOperationalState;
			_nodeAdminState = rhs._nodeAdminState;
			_nodeSelectedState = rhs._nodeSelectedState;
			_xpos = rhs._xpos;
			_ypos = rhs._ypos;
			_bitmapWidth = rhs._bitmapWidth;
			_bitmapHeight = rhs._bitmapHeight;
			_strDisplayName = rhs._strDisplayName;
			_strBitmapName = rhs._strBitmapName;
			_pBitmapCache = rhs._pBitmapCache;
			_opacity = rhs._opacity;
			_origin = rhs._origin;
			_bitmapCacheNormalIndex = rhs._bitmapCacheNormalIndex;
			_pDWriteFactory = rhs._pDWriteFactory;
			_pTextFormat = rhs._pTextFormat;
			_pTextLayout = rhs._pTextLayout;
			_pTextWhiteBrush = rhs._pTextWhiteBrush;
			_dragImageRequired = rhs._dragImageRequired;

			return *this;
		}
		/**
		* RefreshContent
		*
		* Rebuild all content, this will be the result of a resource change based on event such as WM_SIZE
		* Attach to new bitmap
		* Rebuild TextLayout
		*/
		void Node::RefreshContent(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget)
		{
			HRESULT hr = S_OK;
			
			_pTextFormat.Reset();

			hr = _pDWriteFactory->CreateTextFormat(
				L"Calibri",                // Font family name.
				NULL,                       // Font collection (NULL sets it to use the system font collection).
				DWRITE_FONT_WEIGHT_REGULAR,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				8.0f,
				L"en-us",
				&_pTextFormat
				);
			
			if (SUCCEEDED(hr))
			{
				hr = _pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
				hr = _pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
			}

			if (SUCCEEDED(hr))
			{
				hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &_pTextWhiteBrush);
			}
			
			if (SUCCEEDED(hr))
			{
				hr = UpdateNodeText();
			}
			
			if (FAILED(hr))
				throw ::std::runtime_error("Node::RefreshContent D2D Failed.");

			// create bitmap
			_bitmapCacheNormalIndex = _pBitmapCache->GetBitmapIndex(_strBitmapName, 30, 30);
			
			return;
		}
		/**
		*
		*
		*
		*/
		HRESULT Node::UpdateNodeText()
		{
			// calculate text width / height here >>>>>>>>>
			// temp numbers
			//float width = 100;
			//float height = 30;

			float fontSize = _pTextFormat->GetFontSize();		// DPI required 96

			_textStringWidth = fontSize * _strDisplayName.size();
			_textStringHeight = fontSize + 2.0f;
			
			//_textStringWidth /= 2;

			HRESULT hr = _pDWriteFactory->CreateTextLayout(
				_strDisplayName.c_str(),      // The string to be laid out and formatted.
				_strDisplayName.size(),  // The length of the string.
				_pTextFormat.Get(),  // The text format to apply to the string (contains font information, etc).
				_textStringWidth,         // The width of the layout box.
				_textStringHeight,        // The height of the layout box.
				&_pTextLayout  // The IDWriteTextLayout interface pointer.
				);	

			// reset TextLayout width and local reference to layout string width
			DWRITE_OVERHANG_METRICS hangs;
			_pTextLayout->GetOverhangMetrics(&hangs);

			_textStringWidth += hangs.left;
			_textStringWidth += hangs.right;
			_textStringWidth += 2.0f;

			_pTextLayout->SetMaxWidth(_textStringWidth);

			UpdateOrigin();

			return hr;
		}
		/**
		*
		*
		*
		*/
		void Node::Draw(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget>& pRenderTarget, ::NM::ODB::OBJECTUID& groupUID)
		{
			// only draw if this node is in the correct (current) group
			if (groupUID != _groupUID)
				return;

			::NM::NetGraph::ComBitmap pBitmap;
			pBitmap = _pBitmapCache->GetBitmap(_bitmapCacheNormalIndex, _nodeOperationalState, _nodeAdminState, _nodeSelectedState);

			// the bitmap for the node
			pRenderTarget->DrawBitmap(pBitmap.Get(),
				D2D1::RectF(
					(float)(_xpos - (_bitmapWidth / 2)),
					(float)(_ypos - (_bitmapHeight / 2)),
					(float)(_xpos + (_bitmapWidth / 2)),
					(float)(_ypos + (_bitmapHeight / 2))),
				_opacity
				);
		
			// the displayname of the node
			pRenderTarget->DrawTextLayout(_origin, _pTextLayout.Get(), _pTextWhiteBrush.Get());
			return;
		}
		/**
		*
		*
		*
		*/
		bool Node::IsPointInBitmap(POINT &pt)
		{
			// first check its actually in the bitmap rect
			D2D1_RECT_F rectBmp = D2D1::RectF(
				(float)(_xpos - (_bitmapWidth / 2)),
				(float)(_ypos - (_bitmapHeight / 2)),
				(float)(_xpos + (_bitmapWidth / 2)),
				(float)(_ypos + (_bitmapHeight / 2)));
			
			//D2D1::RectF(_xpos - (_bitmapWidth / 2), _ypos - (_bitmapHeight / 2), _xpos + (_bitmapWidth / 2), _ypos + (_bitmapHeight / 2));

			if ((pt.x >= rectBmp.left) && (pt.x <= rectBmp.right))
			{
				if ((pt.y >= rectBmp.top) && (pt.y <= rectBmp.bottom))
				{
					return true;
				}
			}

			return false;

			// then check the pixels
			/*::NM::NetGraph::ComBitmap pBitmap;
			pBitmap = _pBitmapCache->GetBitmap(_bitmapCacheNormalIndex);

			pBitmap->*/
		}
		/**
		*
		*
		*
		*/
		void Node::UpdateOrigin()
		{
			_origin = D2D1::Point2F(
				static_cast<FLOAT>(_xpos - (_textStringWidth/2)),					// / dpiScaleX_),
				static_cast<FLOAT>(_ypos + (_bitmapHeight /2))						 // / dpiScaleY_)
				);
		}
		/**
		*
		*
		*
		*/
		void Node::SetOperationalState(NodeOperationalState operationalState)
		{
			_nodeOperationalState = operationalState;
		}
		/**
		*
		*
		*
		*/
		void Node::SetAdminState(NodeAdminState adminState)
		{
			_nodeAdminState = adminState;
		}
		/**
		*
		*
		*
		*/
		void Node::SetSelectedState(NodeSelectedState selectedState)
		{
			_nodeSelectedState = selectedState;
		}
		/**
		*
		*
		*
		*/
		void Node::SetOpacity(float Opacity)
		{
			_opacity = Opacity;
		}
		/**
		*
		*
		*
		*/
		void Node::SetDragImageStatus(bool bSetDrag)
		{
			_dragImageRequired = bSetDrag;
		}
		/**
		*
		*
		*
		*/
		void Node::SetPos(long xpos, long ypos)
		{
			_xpos = xpos;
			_ypos = ypos;
			UpdateOrigin();
			return;
		}
		/**
		*
		*
		*
		*/
		void Node::GetPos(long &xpos, long &ypos)
		{
			xpos = _xpos;
			ypos = _ypos;
			UpdateOrigin();
			return;
		}
		/**
		*
		*
		*
		*/
		void Node::GetNodePoints(int spatialGridSize, NODEPOINTS& points)
		{
			int x_topleft = abs((_xpos - (_bitmapWidth / 2)) / spatialGridSize);
			int y_topleft = abs((_ypos - (_bitmapHeight / 2)) / spatialGridSize);
			points.insert(::std::make_pair(x_topleft, y_topleft));

			int x_topright = abs((_xpos + (_bitmapWidth / 2)) / spatialGridSize);
			int y_topright = abs((_ypos - (_bitmapHeight / 2)) / spatialGridSize);
			points.insert(::std::make_pair(x_topright, y_topright));

			int x_bottomleft = abs((_xpos - (_bitmapWidth / 2)) / spatialGridSize);
			int y_bottomleft = abs((_ypos + (_bitmapHeight / 2)) / spatialGridSize);
			points.insert(::std::make_pair(x_bottomleft, y_bottomleft));

			int x_bottomright = abs((_xpos + (_bitmapWidth / 2)) / spatialGridSize);
			int y_bottomright = abs((_ypos + (_bitmapHeight / 2)) / spatialGridSize);
			points.insert(::std::make_pair(x_bottomright, y_bottomright));
			return;
		}
		/**
		* GetRect
		*
		* Returns the bounding rectangle of this node drawn
		*/
		D2D1_RECT_F Node::GetRect()
		{
			return ::D2D1::RectF(
				static_cast<float>(_xpos - (max(_bitmapWidth, _textStringWidth)/2)),		// left
				static_cast<float>(_ypos - (_bitmapHeight/2)),								// top
				static_cast<float>(_xpos + (max(_bitmapWidth, _textStringWidth) / 2)),		// right
				static_cast<float>(_ypos + ((_bitmapHeight/2)+_textStringHeight))			// bottom
				);
		}
		/**
		* UpdateValues
		*
		* Not Sure this is used anymore.... specific get/set methods exist
		*/
		void Node::UpdateValues(::NM::ODB::ATTRIBUTENAME& attrName, ::NM::ODB::Value& value)
		{

			if (attrName == L"xpos")
			{
				_xpos = value.Get<::NM::ODB::ODBInt>();
				UpdateOrigin();
			}
			else if (attrName == L"ypos")
			{
				_ypos = value.Get<::NM::ODB::ODBInt>();
				UpdateOrigin();
			}
			else if (attrName == L"imagewidth")
			{
				_bitmapWidth = value.Get<::NM::ODB::ODBInt>();
			}
			else if (attrName == L"imageheight")
			{
				_bitmapHeight = value.Get<::NM::ODB::ODBInt>();
			}
			else if (attrName == L"shortname")
			{
				_strDisplayName = value.Get<::NM::ODB::ODBWString>();
				UpdateNodeText();
			}
			else if (attrName == L"inservice")
			{
				_nodeAdminState = (value.Get<::NM::ODB::ODBBool>() ? NodeAdminState::Enabled : NodeAdminState::Disabled);
			}

			return;
		}

	}
}