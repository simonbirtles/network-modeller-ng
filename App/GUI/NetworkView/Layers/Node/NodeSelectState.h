#pragma once
#include "NodeLayerState.h"

namespace NM
{
	namespace NetGraph
	{
		class NodeSelectState :
			public NodeLayerState
		{
		public:
			NodeSelectState();
			virtual ~NodeSelectState();
			LRESULT ProcessMessage(UINT, WPARAM, LPARAM, NodeLayer*, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& Redraw);
			void    OnRender(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget);

		private:
			LRESULT OnLeftButtonDown(UINT msg, WPARAM wParam, LPARAM lParam, NodeLayer* pNodeLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& Redraw);
			LRESULT OnLeftButtonUp(UINT msg, WPARAM wParam, LPARAM lParam, NodeLayer* pNodeLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& Redraw);
			LRESULT OnMouseMove(UINT msg, WPARAM wParam, LPARAM lParam, NodeLayer* pNodeLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& Redraw);
			LRESULT OnRightButtonUp(UINT msg, WPARAM wParam, LPARAM lParam, NodeLayer* pNodeLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& Redraw);
			LRESULT OnRightButtonDown(UINT msg, WPARAM wParam, LPARAM lParam, NodeLayer* pNodeLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& Redraw);
			LRESULT OnKeyDown(UINT msg, WPARAM wParam, LPARAM lParam, NodeLayer* pNodeLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& Redraw);

			LRESULT OnContextMenu(POINT p, NodeLayer* pNodeLayer);
			LRESULT KeyMoveSelected(NodeLayer* pNodeLayer, WPARAM wParam, LPARAM lParam);

			::std::unique_ptr<::NM::ODB::IObjectDatabase>			 _odb;					// interface to the database



		};


	}
}

