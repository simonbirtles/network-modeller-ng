#pragma once
#include "NodeLayerState.h"		// base class
#include "Interfaces\ObjectDatabaseDefines.h"

#include <vector>
#include <wrl.h>


namespace NM
{
	namespace ODB
	{
		class IObjectDatabase;
	}
	namespace NetGraph
	{
		class Node;
		class NodeLayer;

		class NodeDraggingState : 
			public NodeLayerState
		{
		public:
			NodeDraggingState();
			virtual ~NodeDraggingState();
			LRESULT ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, NodeLayer* pNodeLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& pRedraw);
			void    OnRender(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget);

		private:
			D2D1_RECT_F _dragRect;
			NodeLayer* _pNodeLayer;
			::Microsoft::WRL::ComPtr<IDWriteFactory> pDWriteFactory_;
			::std::unique_ptr<::NM::ODB::IObjectDatabase> _odb;
			NodeLayer::SELECTEDNODES selectedNodes;
			::std::vector<::std::shared_ptr<Node>> _draggingNodes;
			bool _dragNodesCreated;
			POINT _startDragReference;
			POINT _dragImageCenterPointOffset;
			bool _bCursorHidden;
			bool _bDragRectOutofBounds;

			void DeleteDraggingNodes();

			LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam, NodeLayer* pNodeLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget,  bool& pRedraw);
			LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam, NodeLayer* pNodeLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget,  bool& pRedraw);


		};


	}
}
