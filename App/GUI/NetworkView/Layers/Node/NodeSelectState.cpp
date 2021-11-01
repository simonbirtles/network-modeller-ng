#include "stdafx.h"
#include "NodeSelectState.h"
#include "IServiceRegistry.h"							// registry interface
#include "ISelectedObjects.h"							// holds references to currently selected objects application wide
#include "Node.h"

#include "Interfaces\IObjectDatabase.h"					// for client interface to core database 
#include "Interfaces\ObjectDatabaseDefines.h"

extern NM::Registry::IServiceRegistry* reg;

namespace NM
{
	namespace NetGraph
	{
		/**
		*
		*
		*
		*/
		NodeSelectState::NodeSelectState() 
		{
			// if the global registry is not available we are in trouble
			if (!reg)
				throw ::std::runtime_error("Application Registry Not Available, Cannot Continue.");

			// Get The Database Handle and save
			_odb.reset(reinterpret_cast<NM::ODB::IObjectDatabase*>(reg->GetClientInterface(L"ODB")));
			if (!_odb)
				throw ::std::runtime_error("Database Not Available, Cannot Continue.");
		}
		/**
		*
		*
		*
		*/
		NodeSelectState::~NodeSelectState()
		{
		}
		/**
		*
		*
		* Return 0 if we processed msg
		*/
		LRESULT NodeSelectState::ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, NodeLayer* pNodeLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& Redraw)
		{
			LRESULT lResult = 1;
			switch (msg)
			{

			case WM_LBUTTONDBLCLK:
				{
					POINT p = { 0 };
					p.x = GET_X_LPARAM(lParam);
					p.y = GET_Y_LPARAM(lParam);
					::NM::ODB::OBJECTUID uid = pNodeLayer->IsCaretOnObject(p);
					if ((uid != ::NM::ODB::INVALID_OBJECT_UID))
					{
						// must be a node as the group nodes are filtered in NodeLayer process message
						return 0;
					}
				}
				break;

			case WM_LBUTTONDOWN:
				return OnLeftButtonDown(msg, wParam, lParam, pNodeLayer, pRenderTarget, Redraw);
				break;

			case WM_LBUTTONUP:
				return OnLeftButtonUp(msg, wParam, lParam, pNodeLayer, pRenderTarget, Redraw);
				break;

			case WM_MOUSEMOVE:
				return OnMouseMove(msg, wParam, lParam, pNodeLayer, pRenderTarget, Redraw);
				break;

			case WM_RBUTTONDOWN:
				return OnRightButtonDown(msg, wParam, lParam, pNodeLayer, pRenderTarget, Redraw);
				break;

			case WM_RBUTTONUP:
				return OnRightButtonUp(msg, wParam, lParam, pNodeLayer, pRenderTarget, Redraw);
				break;

			case WM_KEYDOWN:
				return OnKeyDown(msg, wParam, lParam, pNodeLayer, pRenderTarget, Redraw);
				break;

			default:
				lResult = 1;
				break;
			}


			return lResult;
		}
		/**
		* OnKeyDown
		*
		* returns 0 if processed
		*/
		LRESULT NodeSelectState::OnKeyDown(UINT msg, WPARAM wParam, LPARAM lParam, NodeLayer* pNodeLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& Redraw)
		{
			LRESULT lResult = 1;

			switch (wParam)
			{
			case VK_RIGHT:
			case VK_LEFT:
			case VK_UP:
			case VK_DOWN:
				lResult = KeyMoveSelected(pNodeLayer, wParam, lParam);
				break;

			default:
				lResult = 1;
			}


			return lResult;
		}
		/**
		*
		*
		*
		*/
		LRESULT NodeSelectState::KeyMoveSelected(NodeLayer* pNodeLayer, WPARAM wParam, LPARAM lParam)
		{
			LRESULT lResult = 0;
			short repeatCount = LOWORD(lParam);
			long moveX = 0;
			long moveY = 0;

			OutputDebugString(L"\nRepeat Count: \t");
			OutputDebugString(::std::to_wstring(repeatCount).c_str());

			// get currently selected nodes
			NodeLayer::SELECTEDNODES selected;
			pNodeLayer->GetSelectedNodes(selected);
			assert(selected.size() != 0);

			switch (wParam)
			{
			case VK_RIGHT:
				moveX = 1 * repeatCount;
				break;

			case VK_LEFT:
				moveX = -1 * repeatCount;
				break;

			case VK_UP:
				moveY = -1 * repeatCount;
				break;

			case VK_DOWN:
				moveY = 1 * repeatCount;
				break;

			default:
				lResult = 1;
				return lResult;
				break;
			}


			::NM::ODB::LockHandle updateLock;
			if (selected.size() > 1)
			{			
				::std::unique_ptr<::NM::ODB::IDatabaseUpdate> updateCache(reinterpret_cast<NM::ODB::IDatabaseUpdate*>(reg->GetClientInterface(L"ObjectUpdateCache")));
				updateLock = updateCache->GetLock();
			}
			::std::unique_ptr<::NM::ODB::IObjectDatabase> odb(reinterpret_cast<NM::ODB::IObjectDatabase*>(reg->GetClientInterface(L"ODB")));
			
			NodeLayer::SELECTEDNODES::iterator it = selected.begin();
			while (it != selected.end())
			{
				NodeLayer::PNODE node = pNodeLayer->GetNode(*it);
				if (node)
				{
					long x = 0; long y = 0; 
					node->GetPos(x, y);

					if (moveX != 0)
					{
						x = max(0, x+moveX);
						odb->SetValue(*it, L"xpos", ::NM::ODB::real_int(static_cast<int>(x)));
					}

					if (moveY != 0)
					{
						y = max(0, y+moveY);
						odb->SetValue(*it, L"ypos", ::NM::ODB::real_int(static_cast<int>(y)));
					}
				}
				++it;
			}
			return lResult;
		}
		/**
		* OnMouseMove
		*
		* The selected state does not deal with mouse moves.. either not for this layer or we move to drag state.
		*/
		LRESULT NodeSelectState::OnMouseMove(UINT msg, WPARAM wParam, LPARAM lParam, NodeLayer* pNodeLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& Redraw)
		{
			LRESULT lResult = 1;
			POINT p = { 0 };
			p.x = GET_X_LPARAM(lParam);
			p.y = GET_Y_LPARAM(lParam);

			if (!IsLeftButtonPressed())
				return 1;

			// left button pressed
			::NM::ODB::OBJECTUID uid = pNodeLayer->IsCaretOnObject(p);
			if (uid == ::NM::ODB::INVALID_OBJECT_UID)
				return 1;	// mouse not over a node, not for us
						
			if (!pNodeLayer->IsSelected(uid))
			{				
				NodeLayer::SELECTEDNODES selected;
				selected.push_back(uid);
				pNodeLayer->SelectNodes(selected);
			}

			// move to drag state
			pNodeLayer->SetState(NodeLayer::LayerState::Drag);
			// return we processed.
			lResult = 0;		
			return lResult;
		}
		/**
		*
		*
		*
		*/
		LRESULT NodeSelectState::OnLeftButtonDown(UINT msg, WPARAM wParam, LPARAM lParam, NodeLayer* pNodeLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& Redraw)
		{
			LRESULT lResult = 1;
			POINT p = { 0 };
			p.x = GET_X_LPARAM(lParam);
			p.y = GET_Y_LPARAM(lParam);
			

			// left button down on node ?
			::NM::ODB::OBJECTUID uid = pNodeLayer->IsCaretOnObject(p);
			if ((uid == ::NM::ODB::INVALID_OBJECT_UID))
			{
				//if (!IsControlKeyPressed())
				//{
				//	// leftbtn down but not on our nodes and control key not pressed...

				//	// deselect all nodes
				//	pNodeLayer->ClearAllSelectedNodes();		
				//	
				//	// return 1 so other layers can process as this was not a click in our layer but as ctrl not pressed we clear any selected

				//	// and trans to idle ???
				//	pNodeLayer->SetState(NodeLayer::LayerState::Idle);

				//	Redraw = true;
				//	//return 1; 
				//}
				//
				//pNodeLayer->_lastMouseDownPoint.x = -1;
				//pNodeLayer->_lastMouseDownPoint.y = -1;
				// no node selected on this click but ctrl pressed so we just ignore and return not dealt with 
				return 1;
			}

			pNodeLayer->_lastMouseDownPoint.x = GET_X_LPARAM(lParam);
			pNodeLayer->_lastMouseDownPoint.y = GET_Y_LPARAM(lParam);

			lResult = 0; // we processed as caret on our node.

			return lResult;
		}
		/**
		* OnRightButtonDown
		*
		*
		*/
		LRESULT NodeSelectState::OnRightButtonDown(UINT msg, WPARAM wParam, LPARAM lParam, NodeLayer* pNodeLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& Redraw)
		{
			LRESULT lResult = 1;
			POINT p = { 0 };
			p.x = GET_X_LPARAM(lParam);
			p.y = GET_Y_LPARAM(lParam);


			// right button down on node ?
			::NM::ODB::OBJECTUID uid = pNodeLayer->IsCaretOnObject(p);
			if ((uid == ::NM::ODB::INVALID_OBJECT_UID))
			{				
				return 1;
			}

			lResult = 0; // we processed as caret on our node although we did nothing, acknowledge this click is for us.

			return lResult;
		}
		/**
		*
		*
		* Return 0 if we processed msg
		*/
		LRESULT NodeSelectState::OnLeftButtonUp(UINT msg, WPARAM wParam, LPARAM lParam, NodeLayer* pNodeLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& Redraw)
		{
			LRESULT lResult = 1;
			POINT p = { 0 };
			p.x = GET_X_LPARAM(lParam);
			p.y = GET_Y_LPARAM(lParam);

			::NM::ODB::OBJECTUID uid = pNodeLayer->IsCaretOnObject(p);
			if ((uid == ::NM::ODB::INVALID_OBJECT_UID))
			{
				pNodeLayer->_lastMouseDownPoint.x = -1;
				pNodeLayer->_lastMouseDownPoint.y = -1;
				return 1;
			}

			lResult = 0;
			NodeLayer::SELECTEDNODES selectedObjects;
			selectedObjects.push_back(uid);

			// if already selected then deselect ONLY if ctrl is pressed otherwise we just leave selected
			if( (pNodeLayer->IsSelected(uid)) && IsControlKeyPressed())
				pNodeLayer->DeselectNodes(selectedObjects);
			else//if(!pNodeLayer->IsSelected(uid))	// otherwise if not selected select it
				pNodeLayer->SelectNodes(selectedObjects);	// selectnodes checks for ctrl key			

			Redraw = true;
			return lResult;
		}
		/**
		*
		*
		* if mouse over a node, select it and call context menu display
		*/
		LRESULT NodeSelectState::OnRightButtonUp(UINT msg, WPARAM wParam, LPARAM lParam, NodeLayer* pNodeLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& Redraw)
		{
			LRESULT lResult = -1;
			POINT p = { 0 };
			p.x = GET_X_LPARAM(lParam);
			p.y = GET_Y_LPARAM(lParam);

			::NM::ODB::OBJECTUID uid = pNodeLayer->IsCaretOnObject(p);
			if ((uid == ::NM::ODB::INVALID_OBJECT_UID))
			{
				pNodeLayer->_lastMouseDownPoint.x = -1;
				pNodeLayer->_lastMouseDownPoint.y = -1;
				return 1;
			}

			lResult = 0;
			// single select only on rightbutton select
			NodeLayer::SELECTEDNODES selectedObjects;
			selectedObjects.push_back(uid);
			pNodeLayer->SelectNodes(selectedObjects);

			// display context menu
			OnContextMenu(p, pNodeLayer);

			// set new state
			//pNodeLayer->SetState(NodeLayer::LayerState::ContextMenu);
			return lResult;
		}
		/**
		*
		*
		*
		*/
		LRESULT NodeSelectState::OnContextMenu(POINT p, NodeLayer* pNodeLayer)
		{
			LRESULT lResult = 1;
			::std::wstring menutext;
			HWND hWnd = pNodeLayer->GetHWND();
			CMenu menu;
			CMenu submenu;

			::std::vector<::NM::ODB::OBJECTUID> selected;
			pNodeLayer->GetSelectedNodes(selected);
			if (selected.size() != 1)
			{
				throw ::std::logic_error("No more than one selected node supported on ContextMenu");
			}

			NodeLayer::PNODE pNode = pNodeLayer->GetNode(selected[0]);
			
			::NM::ODB::OBJECTUID objectUID = pNode->GetNodeUID();
			::std::wstring objectType = _odb->GetObjectTypeName(objectUID);

			if (objectType != L"vertex")
				return 1;
			
			//// create menus
			menu.CreateMenu();
			submenu.CreatePopupMenu();
			menu.AppendMenuW(MF_POPUP | MF_STRING | MF_ENABLED, (UINT_PTR)submenu.m_hMenu, L"NodeLayerMenu");

			// add menu itmes
			NodeAdminState adminState = pNode->GetAdminState();
			submenu.AppendMenuW(MF_STRING, 35100, (adminState == NodeAdminState::Enabled ? L"Disable (Admin State)" : L"Enable (Admin State)"));
			submenu.AppendMenuW(MF_STRING, 35101, L"Delete");
			

			// show popup menu
			POINT screenPoint;
			GetCursorPos(&screenPoint);

			CWnd* pWnd = CWnd::FromHandle(hWnd);
			CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;
			if (pPopupMenu->Create(pWnd, screenPoint.x, screenPoint.y, submenu.m_hMenu, TRUE, FALSE))
			{
				lResult = 1;
			}

			return lResult;

		}
		/**
		*
		*
		*
		*/
		void NodeSelectState::OnRender(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget)
		{

		}


	}
}
