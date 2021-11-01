#include "stdafx.h"
#include "NodeContextState.h"
#include "NodeLayer.h"


namespace NM
{
	namespace NetGraph
	{

		NodeContextState::NodeContextState()
		{
		}


		NodeContextState::~NodeContextState()
		{
		}

		/**
		*
		*
		* Return 0 if we processed msg
		*/
		LRESULT NodeContextState::ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, NodeLayer* pNodeLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& Redraw)
		{

			LRESULT lResult = -1;
			POINT p = { 0 };
			p.x = GET_X_LPARAM(lParam);
			p.y = GET_Y_LPARAM(lParam);


			switch (msg)
			{

			default:
				if (lResult != 0)
				{

					::NM::ODB::OBJECTUID uid = pNodeLayer->IsCaretOnObject(p);
					if ((uid == ::NM::ODB::INVALID_OBJECT_UID))
					{
						pNodeLayer->_lastMouseDownPoint.x = -1;
						pNodeLayer->_lastMouseDownPoint.y = -1;
						pNodeLayer->SetState(NodeLayer::LayerState::Idle);
						return 1;
					}
					else
					{
						pNodeLayer->_lastMouseDownPoint.x = p.x;
						pNodeLayer->_lastMouseDownPoint.y = p.y;
						pNodeLayer->SetState(NodeLayer::LayerState::Select);
					}
				}
				break;
			}
			

			return lResult;
		}
		/**
		*
		*
		*
		*/
		void NodeContextState::OnRender(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget)
		{
		}

	}
}
