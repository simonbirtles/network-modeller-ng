#include "stdafx.h"
#include "GuiTest3.h"
#include "NetworkView.h"


#include "Interfaces\IObjectDatabase.h"					// for client interface to core database 

#include  <string>

extern NM::Registry::IServiceRegistry* reg;

/*********************************************************************************************
*								Messages From Layer Clients
**********************************************************************************************/
/**
* SendLayerMessage
*
* Client layers call this method to send a defined message to NetworkView class (up the chain)
* Returns 0 if successful (lresult)
*/
LRESULT CNetworkView::SendLayerMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = 1;

	switch (message)
	{
		case NVLM_GROUPDISPLAYSET:
		{
			::NM::ODB::OBJECTUID groupUID = *(GUID*)wParam;
			::std::wstring layerString = _odb->GetObjectTypeName(groupUID);
			if (layerString == L"group")
			{
				// set the new active group, will also load previous (if exists) group metadata - transforms
				SetGroupView(groupUID);			
				MessagePipeLine(NVLM_GROUPDISPLAYCHANGE, wParam, 0, true);
				OnRender();
				lResult = 0;
			}
		}
		break;

		default:
			break;
	}

	return lResult;
}
/**
* RenderUpdate
*
* Called from client layers when they have made a change to their layer and require redraw
* not called in reponse to our OnRender call - usually called due to outside data update into
* the client which doesnt pass through this class - which is entirely acceptable.
*/
void CNetworkView::RenderUpdate(::NM::NetGraph::BaseLayer* pLayer)
{
	_bClientRenderUpdateRequired = true;
	m_pLayerUpdateQ.insert(pLayer);
	OnRender();
}
/*********************************************************************************************
*								Messages To Layer Clients
**********************************************************************************************/
/**
*
*
* An application returns zero if it processes this message.
*/
LRESULT CNetworkView::MessagePipeLine(UINT message, WPARAM wParam, LPARAM lParam, bool SendToAllClients)
{
	LRESULT lResult = 1;
	// reverse iterator, we render the layers bottom to top, but send messages like click top to bottom.
	LAYER_RENDER_ORDER::reverse_iterator it = _layerRenderOrder.rbegin();
	while (it != _layerRenderOrder.rend())
	{
		// dont send messages if the layer is not rendering.
		//if ((*it)->ShouldRender() || (message == WM_SIZE))
		//{
			lResult = (*it)->ProcessMessage(message, wParam, lParam);
			if ((lResult == 0) && (!SendToAllClients))	// layer processed this message
			{
				// if this layer has taken focus, let all lower layers know they have lost focus if they had it.
				if (*it != _pLastLayerMessageProcessed)
				{
					// change message to LostFocus to continue sending to lower layers
					message = NVLM_LOSTFOCUS;
					SendToAllClients = true;
					// save/change reference to layer that processed last message
					_pLastLayerMessageProcessed = *it;
					// move to nexy layer, then call that layer and all others to process lost focus
					++it;
					while (it != _layerRenderOrder.rend())
					{
						(*it)->ProcessMessage(message, wParam, lParam);
						++it;
					}
					//return lResult; // from first message return
				}
				//else
				//{
					return lResult; // from first message return
				//}
			}
	//	}

		++it;
	}

	return lResult;  // as we get here only if SendToAllClients=true
}
/**
*
* Overridden from CView
* ALL application command messages are routed through here, suspect CView disptaches all messages to each window
* and its up to the window to grab the relevent nID ones.
* Return : TRUE if the command message was handled; otherwise, FALSE.
*
* [in] nID
*	The command ID as stated by resource, i.e. menu item uID
* [in] nCode
*	Command message category. see https://msdn.microsoft.com/en-us/library/k63bsef7.aspx
* [in, out] pExtra
*	Pointer to a command object.
* [in, out] pHandlerInfo
*	Pointer to a command handler structure.
*
* nCode value				pExtra value
* CN_COMMAND				CCmdUI*
* CN_UPDATE_COMMAND_UI		CCmdUI*
* {others exist but not required for our use - see above link}
*/
BOOL CNetworkView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	/*
	 filter on nCode == CN_COMMAND or CN_UPDATE_COMMAND_UI
	 send these through to layer messages - messagepipeline or new CmdMsgPipeline ????
	*/

	// reserved range for layer command ids
	//if ((nID >= 35000) && (nID < 36000))
	//{

		/*OutputDebugString(L"\n\tCNetworkView::OnCmdMsg::nID\t");
		OutputDebugString(::std::to_wstring(nID).c_str());
		OutputDebugString(L"\tCode:\t");
		OutputDebugString(::std::to_wstring(nCode).c_str());*/

	BOOL bResult = false;
	switch (nID)
	{
	case ID_UPGROUP:
	{
		if (nCode == CN_UPDATE_COMMAND_UI)
		{
			CCmdUI* pCmdUI = (CCmdUI*)pExtra;
			pCmdUI->Enable( (_currentLayerUID == ::NM::ODB::INVALID_OBJECT_UID ? FALSE : TRUE));
			return TRUE;
		}
		if (nCode == CN_COMMAND)
		{
			// move up one group
			CCmdUI* pCmdUI = (CCmdUI*)pExtra;
			assert(_currentLayerUID != ::NM::ODB::INVALID_OBJECT_UID);
			::NM::ODB::OBJECTUID parentLayer = _odb->GetValue(_currentLayerUID, L"group")->Get<::NM::ODB::ODBUID>();
			if (SetGroupView(parentLayer))
			{
				//MessagePipeLine(NVLM_GROUPDISPLAYCHANGE, (WPARAM)&parentLayer, 0, true);
				//OnRender();
			}
			return TRUE;
		}
	}
	break;

	case ID_GLOBAL_GROUP:
	{
		//OutputDebugString(L"\n\t\t ID_GLOBAL_GROUP");
		if (nCode == CN_UPDATE_COMMAND_UI)
		{
			//OutputDebugString(L"\tCN_UPDATE_COMMAND_UI");

			CCmdUI* pCmdUI = (CCmdUI*)pExtra;
			pCmdUI->Enable((_currentLayerUID == ::NM::ODB::INVALID_OBJECT_UID ? FALSE : TRUE));
			return TRUE;
		}
		if (nCode == CN_COMMAND)
		{
			//OutputDebugString(L"\tCN_COMMAND");

			// move to root/Global group
			if (SetGroupView(::NM::ODB::INVALID_OBJECT_UID))
			{
				//MessagePipeLine(NVLM_GROUPDISPLAYCHANGE, (WPARAM)&parentLayer, 0, true);
				//OnRender();
			}
			return TRUE;
		}
	}
	break;

	case ID_BUTTON_TIPS2:
	{
		//OutputDebugString(L"\n\t\t ID_BUTTON_TIPS");

		if (nCode == CN_UPDATE_COMMAND_UI)
		{
			//OutputDebugString(L"\tCN_UPDATE_COMMAND_UI");

			CCmdUI* pCmdUI = (CCmdUI*)pExtra;
			pCmdUI->Enable(TRUE);
			pCmdUI->SetCheck(_bEnableToolTips ? TRUE : FALSE);
			return TRUE;
		}

		if (nCode == CN_COMMAND)
		{

			//OutputDebugString(L"\tCN_COMMAND");
			
			_bEnableToolTips = !_bEnableToolTips;
			if (_bEnableToolTips)
			{
				//OutputDebugString(L"\t ENABLED");
				EnableTrackingToolTips(TRUE);
				StartMouseTrack(0, 0);
			}
			else
			{
				//OutputDebugString(L"\t DISABLED");
				EnableTrackingToolTips(FALSE);
				CancelMouseTrack();
			}


			return TRUE;
		}
	}
	break;

	default:// reverse iterator, we render the layers bottom to top, but send messages like click top to bottom.
		LAYER_RENDER_ORDER::reverse_iterator it = _layerRenderOrder.rbegin();
		while (it != _layerRenderOrder.rend())
		{
			bResult = (*it)->ProcessCommandMessage(nID, nCode, pExtra, pHandlerInfo);;
			if (bResult == TRUE)	// layer processed this message
			{
				return bResult;
			}
			++it;
		}
		break;

	}

		
		

	//}

	// example client code
	//if (nCode == CN_UPDATE_COMMAND_UI)
	//{
	//	if (nID > 9 && nID < 20)
	//	{
	//		CCmdUI* pCmdUI = (CCmdUI*)pExtra;
	//		pCmdUI->Enable(TRUE);
	//		return TRUE;
	//	}
	//}
	//if (nCode == CN_COMMAND)
	//{
	//	if (nID > 9 && nID < 20)
	//	{
	//		CCmdUI* pCmdUI = (CCmdUI*)pExtra;
	//		//pCmdUI->Enable(TRUE);
	//		OutputDebugString(L"\n\tCNetworkView::OnCmdMsg:: Item Clicked.");
	//		return TRUE;
	//	}
	//}
	
	return CView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}
/**
*
* Overridden
* An application returns zero if it processes this message.
*/
LRESULT CNetworkView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = 1;

	switch (message)
	{
	case WM_CREATE:
		//EnableToolTips();
		EnableTrackingToolTips();
		break;

	case WM_INITIALUPDATE:
		ASSERT(this->GetSafeHwnd());
		CreateDirectXResources();
		CreateDiscardableResources();
		CreateRegistryService();
		CreateTrackingToolTip();
		SetGroupView(::NM::ODB::INVALID_OBJECT_UID);		// root group
		UpdateRenderTargetTransforms();			// temp
		OnRender();								// temp
		break;

	case WM_DESTROY:
		CTooltipManager::DeleteToolTip(_pToolTipCtrl);
		_pToolTipCtrl = nullptr;
		break;

	case WM_PAINT:
	{
		///if (!bHaveFocus) return 0;
		PAINTSTRUCT ps;
		HDC hdc = ::BeginPaint(this->GetSafeHwnd(), &ps);
		OnRender();
		::EndPaint(this->GetSafeHwnd(), &ps);
		return 0;	// WM_PAINT usually in normal wndproc returns 0 if processed.
	}
	break;

	case WM_SIZE:
		//OutputDebugString(L"\nNetworkView::WM_SIZE");
		if (m_pSwapChain)
		{
			HRESULT hr = S_OK;
			//DiscardDeviceResources(); // this is called in CreateDiscardableResourcesEx
			hr = CreateDiscardableResourcesEx();
			_bRedraw = true;
			MessagePipeLine(message, wParam, lParam, true);
		}
		return 0;  // WM_SIZE usually in normal wndproc returns 0 if processed.
		break;

	case WM_ERASEBKGND:
		// An application should return nonzero if it erases the background; otherwise, it should return zero.
		return 1; 
		break;

	case WM_LBUTTONDOWN:
		CancelMouseTrack();
		SetCapture();
		{
			POINT p = { 0 };
			::GetCursorPos(&p);
			::ScreenToClient(this->GetSafeHwnd(), &p);
			TransformPoints(p);
			lParam = MAKELPARAM(p.x, p.y);
			lResult = MessagePipeLine(message, wParam, lParam);	
			_bLeftButtonDown = true;
		}
		break;

	case WM_RBUTTONDOWN:
		CancelMouseTrack();
		SetCapture();
		{
			POINT p = { 0 };
			::GetCursorPos(&p);
			::ScreenToClient(this->GetSafeHwnd(), &p);
			TransformPoints(p);
			lParam = MAKELPARAM(p.x, p.y);

			lResult = MessagePipeLine(message, wParam, lParam);
			if (lResult != 0)
			{
				// we use untranslated points as we only care about the delta and not actual point
				POINT p2 = { 0 };
				::GetCursorPos(&p2);
				::ScreenToClient(this->GetSafeHwnd(), &p2);
				_lastMousePosition.x = static_cast<float>(p2.x);
				_lastMousePosition.y = static_cast<float>(p2.y);
				_bRightButtonDown = true;
			}

		}
		break;

	case WM_LBUTTONUP:
		ReleaseCapture();
		{
			POINT p = { 0 };
			::GetCursorPos(&p);
			::ScreenToClient(this->GetSafeHwnd(), &p);
			TransformPoints(p);
			lParam = MAKELPARAM(p.x, p.y);
			_bLeftButtonDown = false;
	
			lResult = MessagePipeLine(message, wParam, lParam);
			StartMouseTrack(p.x, p.y);
		}
		break;


	case WM_RBUTTONUP:
		ReleaseCapture();
		if (_bRightButtonDown)
		{
			_bRightButtonDown = false;
			lResult = 0;
			return lResult;
		}

		{
			POINT p = { 0 };
			::GetCursorPos(&p);
			::ScreenToClient(this->GetSafeHwnd(), &p);
			TransformPoints(p);
			lParam = MAKELPARAM(p.x, p.y);

			lResult = MessagePipeLine(message, wParam, lParam);
			StartMouseTrack(p.x, p.y);
		}
		
		break;

	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
		{
			POINT p = { 0 };
			::GetCursorPos(&p);
			::ScreenToClient(this->GetSafeHwnd(), &p);
			TransformPoints(p);
			lParam = MAKELPARAM(p.x, p.y);
		}
		return MessagePipeLine(message, wParam, lParam);
		break;

	case WM_MOUSELEAVE:
	{
		OutputDebugString(L"\n\t**WM_MOUSELEAVE:TRIGGERED");
		CancelMouseTrack();
		MessagePipeLine(message, wParam, lParam);
		return 0;
	}
	break;

	case WM_MOUSEHOVER:
	{
		OutputDebugString(L"\n\t**WM_MOUSEHOVER:TRIGGERED\t SetTracking=false");

		POINT p = { 0 };
		::GetCursorPos(&p);
		::ScreenToClient(this->GetSafeHwnd(), &p);
		TransformPoints(p);
		lParam = MAKELPARAM(p.x, p.y);
		lResult = MessagePipeLine(message, wParam, lParam);

		if (lResult == 0)
		{
			//POINT p = { 0 };
			::GetCursorPos(&p);
			DisplayTrackingToolTip(p.x, p.y);
		}
		else
		{
			_bTrackingMouse = false;
			_bDisplayingToolTip = false;
			StartMouseTrack(p.x, p.y);
		}
		return lResult;
	}
	break;

	case WM_MOUSEMOVE:
		{
			POINT p = { 0 };
			::GetCursorPos(&p);
			::ScreenToClient(this->GetSafeHwnd(), &p);
			TransformPoints(p);
			lParam = MAKELPARAM(p.x, p.y);

			// if we are not tracking mouse and dont have mouse buttons down
			// start tracking for tooltip
			StartMouseTrack(p.x, p.y);

			if (_bRightButtonDown)
			{
				// we use untranslated points as we only care about the delta and not actual point
				POINT p2 = { 0 };
				::GetCursorPos(&p2);
				::ScreenToClient(this->GetSafeHwnd(), &p2);
				bool updateRequired = false;
				size_t pixelOffset = 1;

				if (abs(_lastMousePosition.x - static_cast<float>(p2.x)) > pixelOffset)
				{
					SetTranslation((static_cast<int>(p2.x) - static_cast<int>(_lastMousePosition.x)), 0);
					_lastMousePosition.x = static_cast<float>(p2.x);
					updateRequired = true;
				}

				if (abs(_lastMousePosition.y - static_cast<float>(p2.y)) > pixelOffset)
				{
					SetTranslation(0, static_cast<int>(p2.y) - static_cast<int>(_lastMousePosition.y));
					_lastMousePosition.y = static_cast<float>(p2.y);				
					updateRequired = true;
				}

				if (updateRequired)
				{
					UpdateRenderTargetTransforms();
					if (m_pSwapChain)
						OnRender();
				}

				lResult = 0;
				
			}
			else
			{
				lResult = MessagePipeLine(message, wParam, lParam);
			}
		}
		break;

	case WM_MOUSEWHEEL:
		{
			short fwKeys = GET_KEYSTATE_WPARAM(wParam);
			short f = GET_WHEEL_DELTA_WPARAM(wParam);			// multiples of 120			

			LRESULT lResult = 1;

			switch (fwKeys)
			{

			case MK_CONTROL:
				// mousewheel + ctrl, scale the window
				SetScale(f < 0 ? -0.10f : 0.10f);				
				UpdateRenderTargetTransforms();
				if (m_pSwapChain)
					OnRender();
				lResult = 0;
				break;

			case MK_SHIFT:
				// just a mousescroll with shift key pressed, scroll (translate) the screen horizontally (x).
				SetTranslation((f < 0 ? -32 : 32), 0);
				UpdateRenderTargetTransforms();
				if (m_pSwapChain)
					OnRender();
				lResult = 0;
				break;


			case 0:
				// just a mousescroll with no key presses, scroll (translate) the screen vertically (y).
				SetTranslation(0, (f < 0 ? -32 : 32));
				UpdateRenderTargetTransforms();
				if (m_pSwapChain)
					OnRender();
				lResult = 0;
				break;

			default:
				lResult = MessagePipeLine(message, wParam, lParam);
				break;
			}
		}
		break;

	

	case WM_INITMENUPOPUP:
	case WM_MENUCOMMAND:
	case WM_COMMAND:
		return MessagePipeLine(message, wParam, lParam);
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_PRIOR:	// page up
			SetTranslation(0, 64);
			UpdateRenderTargetTransforms();
			if (m_pSwapChain)
				OnRender();
			return 0;
			break;

		case VK_NEXT:	// page down
			SetTranslation(0, -64);
			UpdateRenderTargetTransforms();
			if (m_pSwapChain)
				OnRender();
			return 0;
			break;

		case VK_RIGHT:
			lResult = MessagePipeLine(message, wParam, lParam);
			if (lResult != 0)
			{
				SetTranslation(-5, 0);
				UpdateRenderTargetTransforms();
				if (m_pSwapChain)
					OnRender();
				return 0;
			}
			break;

		case VK_LEFT:
			lResult = MessagePipeLine(message, wParam, lParam);
			if (lResult != 0)
			{
				SetTranslation(5, 0);
				UpdateRenderTargetTransforms();
				if (m_pSwapChain)
					OnRender();
				return 0;
			}
			break;

		case VK_UP:
			lResult = MessagePipeLine(message, wParam, lParam);
			if (lResult != 0)
			{
				SetTranslation(0, 5);
				UpdateRenderTargetTransforms();
				if (m_pSwapChain)
					OnRender();
				return 0;
			}
			break;

		case VK_DOWN:
			lResult = MessagePipeLine(message, wParam, lParam);
			if (lResult != 0)
			{
				SetTranslation(0, -5);
				UpdateRenderTargetTransforms();
				if (m_pSwapChain)
					OnRender();
				return 0;
			}
			break;

		default:
			return MessagePipeLine(message, wParam, lParam);
			break;

		}
		break;

	case WM_HSCROLL:
		switch (LOWORD(wParam))
		{
		case SB_LINELEFT:
			SetTranslation(5, 0);
			UpdateRenderTargetTransforms();
			if (m_pSwapChain)
				OnRender();
			return 0;
			break;

		case SB_LINERIGHT:
			SetTranslation(-5, 0);
			UpdateRenderTargetTransforms();
			if (m_pSwapChain)
				OnRender();
			return 0;			
			break;

		//case SB_THUMBPOSITION:
		//	_scrollX = -HIWORD(wParam);
		//	//::SetScrollPos(this->GetSafeHwnd(), SB_HORZ, -_scrollX, TRUE);
		//	UpdateRenderTargetTransforms();
		//	if (m_pSwapChain)
		//		OnRender();
		//	return 0;
		//	break;

		case SB_PAGELEFT:
			SetTranslation(64, 0);
			UpdateRenderTargetTransforms();
			if (m_pSwapChain)
				OnRender();
			return 0;
			break;

		case SB_PAGERIGHT:
			SetTranslation(-64, 0);
			UpdateRenderTargetTransforms();
			if (m_pSwapChain)
				OnRender();
			return 0;
			break;

		case SB_THUMBTRACK:
			_scrollX = -HIWORD(wParam);
			::SetScrollPos(this->GetSafeHwnd(), SB_HORZ, -_scrollX, TRUE);
			UpdateRenderTargetTransforms();
			if (m_pSwapChain)
				OnRender();
			return 0;
			break;


		default:
			break;

		}

		break;

	case WM_VSCROLL:

		switch (LOWORD(wParam))
		{

		case SB_LINEUP:
			SetTranslation(0, 5);
			UpdateRenderTargetTransforms();
			if (m_pSwapChain)
				OnRender();
			return 0;
			break;

		case SB_LINEDOWN:
			SetTranslation(0, -5);
			UpdateRenderTargetTransforms();
			if (m_pSwapChain)
				OnRender();
			return 0;
			break;

		case SB_PAGEUP:
			SetTranslation(0, 64);
			UpdateRenderTargetTransforms();
			if (m_pSwapChain)
				OnRender();
			return 0;
			break;

		case SB_PAGEDOWN:
			SetTranslation(0, -64);
			UpdateRenderTargetTransforms();
			if (m_pSwapChain)
				OnRender();
			return 0;
			break;

		case SB_THUMBTRACK:
			_scrollY = -HIWORD(wParam);
			::SetScrollPos(this->GetSafeHwnd(), SB_VERT, -_scrollY, TRUE);
			UpdateRenderTargetTransforms();
			if (m_pSwapChain)
				OnRender();
			return 0;
			break;

		default:
			break;
		}
	
		break;

	case WM_KILLFOCUS:
		///bHaveFocus = false;
		break;

	case WM_SETFOCUS:
		///bHaveFocus = true;
		break;

	/*case WM_NOTIFY:
	{
		LPNMHDR pnmh = (LPNMHDR)lParam;
		switch (pnmh->code)
		{
		case TTN_SHOW:
			return 0;
			break;

		default:
			break;
		}
	}
	break;*/


	default:
		break;
	}


	//if (message == AFX_WM_UPDATETOOLTIPS)
	//{
	//	OutputDebugString(L"\n AFX_WM_UPDATETOOLTIPS");
	//	OnUpdateToolTips(wParam, lParam);
	//}

	if (lResult != 0)
		return CView::WindowProc(message, wParam, lParam);
	
	return lResult;
}


BOOL CNetworkView::PreTranslateMessage(MSG* pMsg)
{

	//if (pMsg->message == WM_NOTIFY)
	//{
	//	OutputDebugString(L"\n WM_NOTIFY");
	//}
	//switch (pMsg->message)
	//{
	//case WM_KEYDOWN:
	//case WM_SYSKEYDOWN:
	//case WM_LBUTTONDOWN:
	//case WM_RBUTTONDOWN:
	//case WM_MBUTTONDOWN:
	//case WM_LBUTTONUP:
	//case WM_RBUTTONUP:
	//case WM_MBUTTONUP:
	//case WM_MOUSEMOVE:
	//{
	//	_pToolTipCtrl->RelayEvent(pMsg);
	//}
	//	break;

	//default:break;

	//}

	return CView::PreTranslateMessage(pMsg);
}