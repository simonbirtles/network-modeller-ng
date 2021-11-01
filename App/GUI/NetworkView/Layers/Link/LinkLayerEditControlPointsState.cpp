#include "stdafx.h"
#include "LinkLayerEditControlPointsState.h"
#include "Link.h"
#include "LinkLayer.h"
//#include "LinkSpatialHash.h"
#include "NetworkViewSpatialHash.h"

#include "IServiceRegistry.h"							// registry interface
#include "Interfaces\IObjectDatabase.h"					// for client interface to core database 

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
		LinkLayerEditControlPointsState::LinkLayerEditControlPointsState(LinkLayer* pLinkLayer):
			_bMouseButtonDown(false),
			_pLinkLayer(pLinkLayer),
			_controlPoint(0)
		{
			_pSpatialHash = _pLinkLayer->GetLinkSpatialHash();
		}
		/**
		*
		*
		*
		*/
		LinkLayerEditControlPointsState::~LinkLayerEditControlPointsState()
		{
		}
		/**
		*
		*
		* Return 0 if we process this message
		*/
		LRESULT LinkLayerEditControlPointsState::ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, LinkLayer* pLinkLayer, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& Redraw)
		{
			LRESULT lResult = 1;

			POINT p = { 0, 0 };
			p.x = GET_X_LPARAM(lParam);
			p.y = GET_Y_LPARAM(lParam);

			switch (msg)
			{
			case WM_LBUTTONDOWN:
				lResult = OnLeftButtonDown(p);
				break;

			case WM_LBUTTONUP:
				lResult = OnLeftButtonUp(p);
				break;

			case WM_MOUSEMOVE:
				lResult = OnMouseMove(p);
				break;

			default:
				break;
			}

			Redraw = true; // ??

			return lResult;
		}
		/**
		* OnLeftButtonDown
		*
		* Is the left button down on a drawn control point 
		*
		* An application returns zero if it processes this message.
		*/
		LRESULT LinkLayerEditControlPointsState::OnLeftButtonDown(POINT p)
		{
			LRESULT lResult = 1;
			assert(_pLinkEdit);

			_controlPoint = _pLinkEdit->IsPointOnControlPoint(::D2D1::Point2F((float)p.x, (float)p.y));
			if (_controlPoint != 0)
			{
				//set local flag we are on control point, mse down, so entering a drag state
				_bMouseButtonDown = true;
				lResult = 0;
			}
			else
			{
				// clicked outside of links hotspots so cancel.
				EndEditLinkControlPoints();
				lResult = 1;
			}
			return lResult;
		}
		/**
		* OnMouseMove
		*
		* 
		*
		* An application returns zero if it processes this message.
		*/
		LRESULT LinkLayerEditControlPointsState::OnMouseMove(POINT p)
		{
			LRESULT lResult = 1;
			assert(_pLinkEdit);

			
			::D2D1_POINT_2F controlPointA;
			::D2D1_POINT_2F controlPointB;

			// check drag state flag
			if (_bMouseButtonDown && (_controlPoint != 0))
			{
				_pLinkEdit->GetControlPoints(controlPointA, controlPointB);

				// update link with new control position
				if (_controlPoint == 1)
				{
					controlPointA = ::D2D1::Point2F((float)max(5, p.x), (float)max(5, p.y));
				}
				else if (_controlPoint == 2)
				{
					controlPointB = ::D2D1::Point2F((float)max(5, p.x), (float)max(5, p.y));
				}

				_pLinkEdit->SetControlPoints(controlPointA, controlPointB);
			}

			// redraw

			return 0;
		}
		/**
		* OnLeftButtonUp
		*
		* 
		*
		* An application returns zero if it processes this message.
		*/
		LRESULT LinkLayerEditControlPointsState::OnLeftButtonUp(POINT p)
		{
			LRESULT lResult = 1;
			assert(_pLinkEdit);

			// if we were dragging/editing
			//if (_bMouseButtonDown && (_controlPoint != 0))
			//{
			//	EndEditLinkControlPoints();
			//	lResult = 0;
			//}

			if (_bMouseButtonDown)
			{
				_bMouseButtonDown = false;
				lResult = 0;
			}

			return lResult;
		}
		/**
		*
		*
		*
		*/
		void LinkLayerEditControlPointsState::OnRender(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, ::std::shared_ptr<LinkDrawResources> pDrawResources)
		{
			if(_pLinkEdit && _pLink)
				_pLinkEdit->RenderLink(pRenderTarget, pDrawResources);

			return;
		}
		/**
		*
		*
		*
		*/
		void LinkLayerEditControlPointsState::StartEditLinkControlPoints(::std::shared_ptr<Link> pLink)
		{
			// local reference
			_pLink = pLink;

			// copy the link to edit
			_pLinkEdit = ::std::make_shared<Link>(nullptr, ::NM::ODB::INVALID_OBJECT_UID, false);
			*_pLinkEdit = *_pLink;

			// prevent original link from drawing.
			_pLink->SetDrawSuspendedStatus(true);

			// set edit copy to draw editable control points
			_pLinkEdit->SetDrawControlPoints(true);
		
			return;
		}
		/**
		*
		*
		*
		*/
		void LinkLayerEditControlPointsState::EndEditLinkControlPoints()
		{
			// get control points from _pLinkEdit and set same points on _pLink
			::D2D1_POINT_2F controlPointA;
			::D2D1_POINT_2F controlPointB;

			_pLinkEdit->GetControlPoints(controlPointA, controlPointB);
			_pLink->SetControlPoints(controlPointA, controlPointB);

			if (!_pLink->IsInternalLink())
			{
				// Get The Database Handle and save
				::std::unique_ptr<::NM::ODB::IObjectDatabase> _odb;
				_odb.reset(reinterpret_cast<NM::ODB::IObjectDatabase*>(reg->GetClientInterface(L"ODB")));
				_odb->SetValue(_pLink->GetLinkUID(), L"controlpointAxpos", ::NM::ODB::real_int((int)(controlPointA.x)));
				_odb->SetValue(_pLink->GetLinkUID(), L"controlpointAypos", ::NM::ODB::real_int((int)(controlPointA.y)));
				_odb->SetValue(_pLink->GetLinkUID(), L"controlpointBxpos", ::NM::ODB::real_int((int)(controlPointB.x)));
				_odb->SetValue(_pLink->GetLinkUID(), L"controlpointBypos", ::NM::ODB::real_int((int)(controlPointB.y)));
			}

			// set original link to draw
			_pLink->SetDrawSuspendedStatus(false);

			Link::LINKPATHPOINTS points;
			_pLink->GetLinkPathPoints(_pSpatialHash->GetSpatialGridSize(), points);
			_pSpatialHash->UpdateSpatialHashNode(_pLink->GetLinkUID(), points);

			//_pSpatialHash->UpdateSpatialHashNode(_pLink);

			// delete editable link
			_pLinkEdit.reset();

			// clear reference to original link
			_pLink = nullptr;

			// need to flag state change?
			_pLinkLayer->EndCurrentState();

			_controlPoint = 0;
			return;
		}


		// ns
	}
}