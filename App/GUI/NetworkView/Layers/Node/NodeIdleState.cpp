#include "stdafx.h"
#include "NodeIdleState.h"
#include "IServiceRegistry.h"							// registry interface
#include "ISelectedObjects.h"							// holds references to currently selected objects application wide

extern NM::Registry::IServiceRegistry* reg;

using ::NM::ODB::OBJECTUID;

namespace NM
{
	namespace NetGraph
	{

		/**
		*
		*
		*
		*/
		NodeIdleState::NodeIdleState():
			_lastMousePoint({0, 0}),
			_lastLeftButtonStateDown(false)
		{
		}
		/**
		*
		*
		*
		*/
		NodeIdleState::~NodeIdleState()
		{
		}
		/**
		*
		*
		* Return 0 if we processed msg
		*/
		LRESULT NodeIdleState::ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, NodeLayer* pNodeLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& Redraw)
		{
			POINT p = { 0 };
			p.x = GET_X_LPARAM(lParam);
			p.y = GET_Y_LPARAM(lParam);

			LRESULT lResult = 1;
			switch (msg)
			{
			case WM_LBUTTONDBLCLK:
			{				
				::NM::ODB::OBJECTUID uid = pNodeLayer->IsCaretOnObject(p);
				if ((uid != ::NM::ODB::INVALID_OBJECT_UID))
				{
					return 0;
				}
				break;
			}

			case WM_LBUTTONDOWN:
				return OnLeftButtonDown(msg, wParam, lParam, pNodeLayer, pRenderTarget, Redraw);
				break;

			case WM_LBUTTONUP:
				break;

			case WM_MOUSEMOVE:
				break;

			case WM_RBUTTONDOWN:
				// save the current mouse pointer if we are on a node and change state to Select
				::NM::ODB::OBJECTUID uid = pNodeLayer->IsCaretOnObject(p);
				if ((uid == ::NM::ODB::INVALID_OBJECT_UID))
				{
					lResult = 1;
				}
				else
				{
					pNodeLayer->_lastMouseDownPoint.x = p.x;
					pNodeLayer->_lastMouseDownPoint.y = p.y;
					pNodeLayer->SetState(NodeLayer::LayerState::Select);
					lResult = 0; // we processed.
				}
				break;


			default:
				lResult = 1;
				break;
			}


			return lResult;
		}
		/**
		*
		*
		*
		*/
		void NodeIdleState::OnRender(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget)
		{

		}
		/**
		*
		*
		* If btndown on a node, dont select but change state to Select, two things can happen at this stage
		* 1. BtnUp - which is a select of the object
		* 2. BtnDown(no change) and MouseMove which transistions to a Drag state
		* Both will be handled by Select state, so as long as we are btn down on a node, then trans to select state, but 
		* selection only actually happens on btnup or mouse move.
		*
		* Return 0 is processed otherwise non-zero
		*/
		LRESULT NodeIdleState::OnLeftButtonDown(UINT msg, WPARAM wParam, LPARAM lParam, NodeLayer* pNodeLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& Redraw)
		{
			LRESULT lResult = 1;
			POINT p = { 0 };
			p.x = GET_X_LPARAM(lParam);
			p.y = GET_Y_LPARAM(lParam);

			::NM::ODB::OBJECTUID uid = pNodeLayer->IsCaretOnObject(p);
			if (uid == ::NM::ODB::INVALID_OBJECT_UID)
			{
				return 1;	// not a node selected.. let other layers process.
			}

			pNodeLayer->_lastMouseDownPoint.x = p.x;
			pNodeLayer->_lastMouseDownPoint.y = p.y;
			pNodeLayer->SetState(NodeLayer::LayerState::Select);
			lResult = 0; // we processed.

			//::std::unique_ptr<NM::ISelectedObjects> selectedObjects(reinterpret_cast<NM::ISelectedObjects*>(reg->GetClientInterface(L"SELECTEDOBJECTS")));
			//selectedObjects->SetMultiSelect(IsControlKeyPressed());
			//selectedObjects->Select(uid);

			//_lastMousePoint = p;
			//_lastLeftButtonStateDown = true;


			return lResult;
		}
		/**
		*
		*
		* Return 0 is processed otherwise non-zero
		*/
		LRESULT NodeIdleState::OnLeftButtonUp(UINT msg, WPARAM wParam, LPARAM lParam, NodeLayer* pNodeLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& Redraw)
		{
			LRESULT lResult = 1;
			POINT p = { 0 };
			p.x = GET_X_LPARAM(lParam);
			p.y = GET_Y_LPARAM(lParam);

			pNodeLayer->_lastMouseDownPoint.x = -1;
			pNodeLayer->_lastMouseDownPoint.y = -1;

			return lResult;
		}

	}
}