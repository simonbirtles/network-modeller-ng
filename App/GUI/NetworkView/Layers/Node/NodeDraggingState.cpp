#include "stdafx.h"
#include "NodeDraggingState.h"
#include "NodeLayer.h"
#include "Node.h"

#include "IServiceRegistry.h"							// registry interface
#include "ISelectedObjects.h"							// holds references to currently selected objects application wide
#include "Interfaces\IObjectDatabase.h"					// for client interface to core database 
#include "Interfaces\ObjectDatabaseDefines.h"

#include <iterator>

extern NM::Registry::IServiceRegistry* reg;

using ::NM::ODB::OBJECTUID;
using ::NM::ODB::UPVALUE;
using ::NM::ODB::INVALID_OBJECT_UID;
using ::NM::ODB::real_int;
using ::NM::ODB::real_string;

namespace NM
{
	namespace NetGraph
	{
		/**
		* NodeDraggingState
		*
		*
		*/
		NodeDraggingState::NodeDraggingState() :
			_dragNodesCreated(false),
			_startDragReference({ -1, -1 }),
			_dragImageCenterPointOffset({0,0}),
			_bCursorHidden(false),
			_bDragRectOutofBounds(false)
		{
			// if the global registry is not available we are in trouble
			if (!reg)
				throw ::std::runtime_error("Application Registry Not Available, Cannot Continue.");

			// Get The Database Handle and save
			_odb.reset(reinterpret_cast<NM::ODB::IObjectDatabase*>(reg->GetClientInterface(L"ODB")));
			if (!_odb)
				throw ::std::runtime_error("NodeDraggingState::Database not available.");

			// create direct write factory
			HRESULT hr = DWriteCreateFactory(
				DWRITE_FACTORY_TYPE_SHARED,
				__uuidof(IDWriteFactory),
				reinterpret_cast<IUnknown**>(pDWriteFactory_.GetAddressOf())
				);
		}
		/**
		* ~NodeDraggingState
		*
		*
		*/
		NodeDraggingState::~NodeDraggingState()
		{
			DeleteDraggingNodes();
		}
		/**
		* DeleteDraggingNodes
		*
		*
		*/
		void NodeDraggingState::DeleteDraggingNodes()
		{
			for (size_t t = 0;t < _draggingNodes.size();++t)
			{
				_draggingNodes[t].reset();
				//_draggingNodes[t] = nullptr;
			}
			_draggingNodes.clear();
		}
		/**
		* ProcessMessage
		*
		* An application returns zero if it processes a message.
		*/
		LRESULT NodeDraggingState::ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, NodeLayer* pNodeLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& pRedraw)
		{
			LRESULT lResult = -1;

			switch (msg)
			{
			case WM_LBUTTONDOWN:
				_startDragReference.x = GET_X_LPARAM(lParam);
				_startDragReference.y = GET_Y_LPARAM(lParam);
				return 0;
				break;

			case WM_LBUTTONUP:
				// end of dragging state
				lResult = OnLButtonUp(wParam, lParam, pNodeLayer, pRenderTarget, pRedraw);
				break;

			case WM_MOUSEMOVE:
				// check LButtonDown and process
				lResult = OnMouseMove(wParam, lParam, pNodeLayer, pRenderTarget, pRedraw);
				break;

			default:
				lResult = -1;
				break;
			}

			return lResult; // not dealt with
		}
		/**
		* OnMouseMove
		*
		* An application returns zero if it processes this message.
		*/
		LRESULT NodeDraggingState::OnMouseMove(WPARAM wParam, LPARAM lParam, NodeLayer* pNodeLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& pRedraw)
		{
			assert(pRenderTarget);
			// save parent node layer ptr for use by Draw		
			_pNodeLayer = pNodeLayer;
			
			// just started dragging, but we allow a few pixels before we start,
			// by the time this function has been called twice, at least 2 px will have been
			// moved over.
			if ((_startDragReference.x == -1) && (_startDragReference.y == -1))
			{
				// save first mouse position as reference to moving objects
				_startDragReference.x = pNodeLayer->_lastMouseDownPoint.x; 
				_startDragReference.y = pNodeLayer->_lastMouseDownPoint.y; 

				// todo update from static to resource
				//HCURSOR hCurs1 = LoadCursorFromFile(L"C:\\Users\\Simon\\Documents\\Personal\\Source Code\\Projects\\NetworkModelv2\\Exe\\res\\startdragcur.cur");
				//::SetCursor(hCurs1);

				//return 0;
			}

			// wait for a few px move before starting dragging
			//if (!_dragNodesCreated)
			//{
			//	if ((abs(_startDragReference.x - GET_X_LPARAM(lParam)) < 1) && (abs(_startDragReference.y - GET_Y_LPARAM(lParam)) < 1))
			//	{
			//		//OutputDebugString(L"\nDragging < px, waiting for more movement");
			//		return 0;
			//	}
			//}

			// moved a few px, so create the dragging icons and load cursor NSEW & pointer
			if (!_dragNodesCreated)
			{
				// set drag rect default with top, left at a max which will never be the case, see min() funcs
				_dragRect = { 100000,100000,0,0 };

				//OutputDebugString(L"\nNodeDraggingState::OnMouseMove\tCreateDragNodes");
				POINT pt{ _startDragReference.x, _startDragReference.y };
				::NM::ODB::OBJECTUID dragUID = pNodeLayer->IsCaretOnObject(pt);

				// get the currently selected objects to drag (in NodeLayer)
				selectedNodes.clear();
				pNodeLayer->GetSelectedNodes(selectedNodes);

				// create copies for dragging
				NodeLayer::SELECTEDNODES::iterator it = selectedNodes.begin();
				while (it != selectedNodes.end())
				{
					NodeLayer::PNODE node = pNodeLayer->CopyNode(*it);
					assert(node);
					node->SetOpacity(0.5f);
					if (*it == dragUID)
					{
						node->SetDragImageStatus(true);
						long x = 0;
						long y = 0;
						node->GetPos(x, y);
						size_t width = 0;
						size_t height = 0;
						node->GetSize(width, height);

						_dragImageCenterPointOffset.x = (_startDragReference.x - x);
						_dragImageCenterPointOffset.y = (_startDragReference.y - y);						
						//_dragImageCenterPointOffset.x = (GET_X_LPARAM(lParam) - x);
						//_dragImageCenterPointOffset.y = (GET_Y_LPARAM(lParam) - y);
					}
					_draggingNodes.push_back( node );

					// update drag rect - we are creating one rectange which contains all the nodes being dragged.
					D2D1_RECT_F nodeRect = node->GetRect();
					_dragRect.left = min(_dragRect.left, nodeRect.left);
					_dragRect.top = min(_dragRect.top, nodeRect.top);
					_dragRect.right = max(_dragRect.right, nodeRect.right);
					_dragRect.bottom = max(_dragRect.bottom, nodeRect.bottom);

					++it;
				}
				_dragNodesCreated = true;
				return 0;
			}
			
			//::std::wstring currMousePoint = L"\n\tCurrent Mouse Point (Rel. Client Area)\t x=" + ::std::to_wstring(GET_X_LPARAM(lParam)) + L"\ty=" + ::std::to_wstring(GET_Y_LPARAM(lParam));
			//OutputDebugString(currMousePoint.c_str());

			// update the dragging object(s) position			
			long move_x = (GET_X_LPARAM(lParam) - _startDragReference.x);
			long move_y = (GET_Y_LPARAM(lParam) - _startDragReference.y);

			// check for out of bounds and display no entry cursor
			_dragRect.left += move_x;
			_dragRect.top += move_y;
			_dragRect.right += move_x;
			_dragRect.bottom += move_y;

			//::std::wstring strDebug = L"\nDrag Rect:\t left=" + ::std::to_wstring(_dragRect.left);
			//strDebug += L"\t top=" + ::std::to_wstring(_dragRect.top);
			//strDebug += L"\t right=" + ::std::to_wstring(_dragRect.right);
			//strDebug += L"\t bottom=" + ::std::to_wstring(_dragRect.bottom);
			//strDebug += L"\t Mouse(x,y)=(" + ::std::to_wstring(GET_X_LPARAM(lParam)) + L"," + ::std::to_wstring(GET_Y_LPARAM(lParam)) + L")";
			//OutputDebugString(strDebug.c_str());

			// get transform matrix
			D2D1::Matrix3x2F matrix;
			pRenderTarget->GetTransform(&matrix);
			matrix.Invert();	
			// get rendertarget size
			D2D1_SIZE_F clientRenderTargetSize = pRenderTarget->GetSize();
			// transform rendertarget width/height (these are the pixels at max width/height)
			D2D1_POINT_2F clientSizePoint = matrix.TransformPoint(::D2D1::Point2F(clientRenderTargetSize.width, clientRenderTargetSize.height));
			// transform start/end points, (top,left) - from 0,0 to current top,left
			D2D1_POINT_2F clientSizeTopLeftPoint = matrix.TransformPoint(::D2D1::Point2F(0.0f, 0.0f));
			// final transformed client rect
			D2D1_RECT_F clientSizeF = ::D2D1::RectF(clientSizeTopLeftPoint.x, clientSizeTopLeftPoint.y, clientSizePoint.x, clientSizePoint.y);
			
			//strDebug = L"\nclientSizeF Rect:\t left=" + ::std::to_wstring(clientSizeF.left);
			//strDebug += L"\t top=" + ::std::to_wstring(clientSizeF.top);
			//strDebug += L"\t right=" + ::std::to_wstring(clientSizeF.right);
			//strDebug += L"\t bottom=" + ::std::to_wstring(clientSizeF.bottom);
			//strDebug += L"\t Mouse(x,y)=(" + ::std::to_wstring(GET_X_LPARAM(lParam)) + L"," + ::std::to_wstring(GET_Y_LPARAM(lParam)) + L")";
			//OutputDebugString(strDebug.c_str());

			// if drag rect outside client bounds
			if ((_dragRect.left < clientSizeF.left) ||
				(_dragRect.top < clientSizeF.top) ||
				(_dragRect.right > (clientSizeF.right) ||
				(_dragRect.bottom > clientSizeF.bottom)))
			{
				// flag
				_bDragRectOutofBounds = true;
				
				if (_dragRect.left < clientSizeF.left)
				{
					// stop rect moving left
					float diffx = abs(_dragRect.left - clientSizeF.left);
					_dragRect.left = clientSizeF.left; 
					_dragRect.right += diffx;
					move_x += static_cast<long>(diffx);
				}

				if (_dragRect.right > clientSizeF.right)
				{
					// stop rect moving right
					float diffx = abs(_dragRect.right - clientSizeF.right);
					_dragRect.right = clientSizeF.right;
					_dragRect.left -= diffx;
					move_x += move_x -= static_cast<long>(diffx);
				}
				
				if (_dragRect.top < clientSizeF.top)
				{
					// stop rect moving upwards
					float diffy = abs(_dragRect.top - clientSizeF.top);
					_dragRect.top = clientSizeF.top; 
					_dragRect.bottom += diffy;
					move_x += move_y += static_cast<long>(diffy);
				}

				if (_dragRect.bottom > clientSizeF.bottom)
				{
					// stop rect moving downwards
					float diffy = abs(_dragRect.bottom - clientSizeF.bottom);
					_dragRect.bottom = clientSizeF.bottom;
					_dragRect.top -= diffy;
					move_x += move_y -= static_cast<long>(diffy);
				}
				
				// show cursor if not already shown
				if (_bCursorHidden)
				{
					HCURSOR hCurs2 = LoadCursor(NULL, IDC_NO);
					::SetCursor(hCurs2);
					::ShowCursor(TRUE);
					_bCursorHidden = false;
				}
			}
			// otherwise we are in bounds and if the cursor is not hidden, hide it so we can draw our own drag cursor.
			else if (!_bCursorHidden)
			{
				//OutputDebugString(L"\nHiding Cursor - Valid Bounds");
				HCURSOR hCurs2 = LoadCursor(NULL, IDC_ARROW);
				::SetCursor(hCurs2);
				::ShowCursor(FALSE);
				_bCursorHidden = true;
				_bDragRectOutofBounds = false;
			}
						
			// update last drag reference points.
			_startDragReference.x += move_x;
			_startDragReference.y += move_y;

			::std::vector<::std::shared_ptr<Node>>::iterator it = _draggingNodes.begin();
			while (it != _draggingNodes.end())
			{
				long xpos = 0;
				long ypos = 0;
				(*it)->GetPos(xpos, ypos);
				(*it)->SetPos(xpos + move_x, ypos + move_y);
				++it;
			}
			pRedraw = true;
			return 0;
		}
		/**
		* OnLButtonUp
		*
		* An application returns zero if it processes this message.
		*/
		LRESULT NodeDraggingState::OnLButtonUp(WPARAM wParam, LPARAM lParam, NodeLayer* pNodeLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& pRedraw)
		{
			bool boundsValid = false;

			if( (_dragRect.left >= 0) && (_dragRect.top >= 0))
			{
				// update actual objects that were dragged x,y pos
				::NM::ODB::LockHandle updateLock;
				::NM::ODB::IDatabaseUpdate* updateCache = reinterpret_cast<NM::ODB::IDatabaseUpdate*>(reg->GetClientInterface(L"ObjectUpdateCache"));
				if (updateCache)
				{
					updateLock = updateCache->GetLock();
				}
				delete updateCache;
				updateCache = nullptr;

				for (size_t t = 0; t < selectedNodes.size(); ++t)
				{
					long xpos = 0;
					long ypos = 0;
					_draggingNodes[t]->GetPos(xpos, ypos);
					_odb->SetValue(selectedNodes[t], L"xpos", real_int(xpos));
					_odb->SetValue(selectedNodes[t], L"ypos", real_int(ypos));
				}

			}

			// render & signal end of dragging state
			_dragNodesCreated = false;
			DeleteDraggingNodes();
			_startDragReference = { -1, -1 };
			selectedNodes.clear();

			if (_bCursorHidden)
			{
				::ShowCursor(TRUE);
				_bCursorHidden = false;
			}

			_bDragRectOutofBounds = false;
			pNodeLayer->SetState(NodeLayer::LayerState::Select);
			pRedraw = true;	// would cause two draws as we are setting DB values which cause a dbupdate to go to networkview anyway

			return 0;
		}
		/**
		* OnRender
		*
		* render dragging nodes
		*/
		void NodeDraggingState::OnRender(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget)
		{
			::std::shared_ptr<BitmapCache> pBmpCache = _pNodeLayer->GetBitmapCache();
			::std::vector<::std::shared_ptr<Node>>::iterator nodesit = _draggingNodes.begin();
			::NM::ODB::OBJECTUID currentGroup = _pNodeLayer->GetCurrentLayerUID();

			while (nodesit != _draggingNodes.end())
			{
				(*nodesit)->Draw(pRenderTarget, currentGroup);

				if ((*nodesit)->GetDragImageStatus())
				{

					if (!_bDragRectOutofBounds)
					{
						// draw drag overlay bitmap
						// get the overlay bitmap for dragging from the bitmap cache
						::Microsoft::WRL::ComPtr<ID2D1Bitmap> overlayBmp = pBmpCache->GetDragOverlayBitmap();
						// get the position of the node image that we have the mouse pointer over that
						// will represent the mouse position reference point.
						long x = 0, y = 0;
						(*nodesit)->GetPos(x, y);
						// draw the overlay bitmap on this node position so it looks like the 
						// cursor is actually on the image, the real cursor will be hidden 
						// during dragging.

						::D2D1::Matrix3x2F m;
						pRenderTarget->GetTransform(&m);
						m.Invert();
						// scale image manually
						size_t imageWidth = static_cast<size_t>(16 * m._11);
						size_t imageHeight = static_cast<size_t>(16 * m._22);

						pRenderTarget->DrawBitmap(overlayBmp.Get(), ::D2D1::RectF(
							static_cast<float>((x + _dragImageCenterPointOffset.x) - imageWidth),			// left
							static_cast<float>((y + _dragImageCenterPointOffset.y) - imageHeight),			// top
							static_cast<float>((x + _dragImageCenterPointOffset.x) + imageWidth),			// right
							static_cast<float>((y + _dragImageCenterPointOffset.y) + imageHeight)			// bottom
							), 1.0f);
					}
				}			
				++nodesit;
			}

#ifdef DEBUG
			//// test- draw the drag rect
			//::Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> rectbrush;
			//pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), rectbrush.GetAddressOf());
			//pRenderTarget->DrawRectangle(_dragRect, rectbrush.Get(), 0.5f);
			//rectbrush.Reset();
#endif 
			return;
		}
// ns
	}
}
