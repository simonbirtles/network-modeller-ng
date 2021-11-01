#include "stdafx.h"
#include "Link.h"
#include "LinkDrawResources.h"

namespace NM
{
	namespace NetGraph
	{
		/**
		*
		*
		*
		*/
		Link::Link(::Microsoft::WRL::ComPtr<ID2D1Factory> pD2DFactory, ::NM::ODB::OBJECTUID objectUID, bool internalLink) :
			_pD2DFactory(pD2DFactory),
			_objectUID(objectUID),
			_bDrawSuspended(false),
			_bDrawControlPoints(false),
			_bDrawLinear(true),
			_midpoint({ 0,0 }),
			_fWidth(1.5f),
			_bSelected(false),
			_bInternalLink(internalLink),
			_linkUserColorNormalState(::D2D1::ColorF(::D2D1::ColorF::White)),
			_bezierSamplePoints(150)
		{
		}
		/**
		*
		*
		*
		*/
		Link::~Link()
		{
			_pPathGeometry.Reset();
			////_nodebrushSelected.Reset();
			////_nodebrushControlPtsLine.Reset();
			////_nodebrushRed.Reset();
			_nodebrushNormal.Reset();
			////_nodebrushStateDown.Reset();
			////_nodebrushStateUnknown.Reset();
		}
		/**
		* RenderLink
		*
		* Need to remove the brushes and create once until required to recreate..
		*/
		void Link::RenderLink(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> &pRenderTarget, ::std::shared_ptr<LinkDrawResources> pDrawResources)
		{
			if (_bDrawSuspended) return;

			pRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
			
			// check if not already created
			if	(!_nodebrushNormal)
			{
				RefreshContent(pRenderTarget);
			}

			if (_bDrawControlPoints)
			{
				pRenderTarget->DrawLine(_endPointA, _controlPointA, pDrawResources->_nodebrushControlPtsLine.Get());
				pRenderTarget->DrawLine(_endPointB, _controlPointB, pDrawResources->_nodebrushControlPtsLine.Get());
				pRenderTarget->FillEllipse(::D2D1::Ellipse(_controlPointA, 5, 5), pDrawResources->_nodebrushRed.Get());
				pRenderTarget->DrawEllipse(::D2D1::Ellipse(_controlPointA, 5, 5), pDrawResources->_nodebrushControlPtsLine.Get());
				pRenderTarget->FillEllipse(::D2D1::Ellipse(_controlPointB, 5, 5), pDrawResources->_nodebrushRed.Get());
				pRenderTarget->DrawEllipse(::D2D1::Ellipse(_controlPointB, 5, 5), pDrawResources->_nodebrushControlPtsLine.Get());
			}

			if (!_pPathGeometry.Get())
				RefreshGeometry();

			if (_bSelected)
			{
				pRenderTarget->DrawGeometry(
					_pPathGeometry.Get(),
					pDrawResources->_nodebrushSelected.Get(),
					4.0f + _fWidth);
			}

			::Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pBrush;
			if (_bDrawControlPoints)
				pBrush = pDrawResources->_nodebrushControlPtsLine.Get();
			else if (_linkOperationalState == LinkOperationalState::Up)
				pBrush = _nodebrushNormal.Get();
			else if (_linkOperationalState == LinkOperationalState::Down)
				pBrush = pDrawResources->_nodebrushStateDown.Get();
			else if (_linkOperationalState == LinkOperationalState::Unknown)
				pBrush = pDrawResources->_nodebrushStateUnknown.Get();

			pRenderTarget->DrawGeometry(_pPathGeometry.Get(), pBrush.Get(), _fWidth);
			

			if (_bInternalLink)
			{
				D2D1_POINT_2F midpoint = GetBezierMidPoint();
				pRenderTarget->FillEllipse(::D2D1::Ellipse(midpoint, 3, 3), _nodebrushNormal.Get());
			}
			
			pBrush.Reset();
			return;
		}
		/**
		*
		*
		*
		*/
		void Link::RefreshContent(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> &pRenderTarget)
		{
			_nodebrushNormal.Reset();
			pRenderTarget->CreateSolidColorBrush(_linkUserColorNormalState, _nodebrushNormal.GetAddressOf());
			return;
		}
		/**
		*
		*
		*
		*/
		void Link::RefreshGeometry()
		{
			_pPathGeometry.Reset();

			HRESULT hr = _pD2DFactory->CreatePathGeometry(_pPathGeometry.GetAddressOf());

			// Write to the path geometry using the geometry sink.
			::Microsoft::WRL::ComPtr<ID2D1GeometrySink> pSink;

			hr = _pPathGeometry->Open(pSink.GetAddressOf());

			if (SUCCEEDED(hr))
			{
				pSink->BeginFigure(_endPointA, D2D1_FIGURE_BEGIN_HOLLOW);
				pSink->AddBezier(D2D1::BezierSegment(_controlPointA, _controlPointB, _endPointB));
				pSink->EndFigure(D2D1_FIGURE_END_OPEN);
				pSink->Close();
			}
			pSink.Reset();

			return;
		}
		/**
		*
		*
		*
		*/
		size_t Link::IsPointOnControlPoint(D2D1_POINT_2F &point)
		{
			if (_bDrawSuspended) return 0;

			float buffersize = 5 + _fWidth;

			if ((point.x >= _controlPointA.x - buffersize) && (point.x <= _controlPointA.x + buffersize))
			{
				if ((point.y >= _controlPointA.y - buffersize) && (point.y <= _controlPointA.y + buffersize))
				{
					return 1;
				}
			}

			if ((point.x >= _controlPointB.x - buffersize) && (point.x <= _controlPointB.x + buffersize))
			{
				if ((point.y >= _controlPointB.y - buffersize) && (point.y <= _controlPointB.y + buffersize))
				{
					return 2;
				}
			}

			return 0;
		}
		/**
		*
		*
		*
		*/
		bool Link::IsPointOnLine(D2D1_POINT_2F &point)
		{
			CString ot;
			ot.Format(L"\nEndpoints: %d,%d\t%d,%d\tPoint %d,%d", int(_endPointA.x), int(_endPointA.y), int(_endPointB.x), int(_endPointB.y), int(point.x), int(point.y));
			OutputDebugString(ot);
			if (_bDrawSuspended) return false;
			BOOL contains = FALSE;
			_pPathGeometry->StrokeContainsPoint(::D2D1::Point2F(point.x, point.y), 5.0f + _fWidth, 0, ::D2D1::Matrix3x2F::Identity(), &contains);
			return (contains == TRUE ? true : false);
		}
		/**
		*
		*
		*
		*/
		void Link::SetEndPoints(D2D1_POINT_2F &pointA, D2D1_POINT_2F &pointB)
		{
			_endPointA = pointA;
			_endPointB = pointB;
			
			// calc the midpoint between the verticies
			_midpoint = ::D2D1::Point2F(((_endPointA.x + _endPointB.x) / 2), ((_endPointA.y + _endPointB.y) / 2));
			
			if (_bDrawLinear)
				ResetControlPoints();

			RefreshGeometry();
		}
		/**
		*
		*
		*
		*/
		void Link::GetEndPoints(D2D1_POINT_2F &pointA, D2D1_POINT_2F &pointB)
		{
			pointA = _endPointA;
			pointB = _endPointB;
		}
		/**
		*
		*
		*
		*/
		void Link::SetControlPoints(D2D1_POINT_2F &controlPointA, D2D1_POINT_2F &controlPointB)
		{
			if ((controlPointA.x == -1) &&
				(controlPointA.y == -1) &&
				(controlPointB.x == -1) &&
				(controlPointA.y == -1))
			{
				ResetControlPoints();
			}
			else if ((controlPointA.x == _midpoint.x) &&
				(controlPointA.y == _midpoint.y) &&
				(controlPointB.x == _midpoint.x) &&
				(controlPointA.y == _midpoint.y))
			{
				ResetControlPoints();
			}
			else
			{
				_bDrawLinear = false;
				_controlPointA = controlPointA;
				_controlPointB = controlPointB;
				RefreshGeometry();
			}
			return;
		}
		/**
		*
		*
		*
		*/
		void Link::GetControlPoints(D2D1_POINT_2F &pointA, D2D1_POINT_2F &pointB)
		{
			pointA = _controlPointA;
			pointB = _controlPointB;
		}
		/**
		* ResetControlPoints
		*
		* Prob need to save the midpoints on node pos change for the draw func to use if the linear flag is set.
		*/
		void Link::ResetControlPoints()
		{

			// slope
			float m = static_cast<float>(
				(_endPointB.y - _endPointA.y) /
				((_endPointB.x - _endPointA.x) == 0 ? 0.0001f : (_endPointB.x - _endPointA.x)));

			// perpendicular slope
			/*m = -1 / m;*/

			// constant
			//float c = static_cast<float>(_midpoint.y - (m * _midpoint.x));

			// angle
			float angle = atan(m)*-1;

			// for each endpoint calculate the distance and x,y for the control point from the endpoint
			float radius = 40.0f;

			if (_endPointB.x < _endPointA.x)
			{
				// NodeA Endpoint
				_controlPointA.x = _endPointA.x - cos(angle)*radius;
				_controlPointA.y = sin(angle)*radius + _endPointA.y;

				//NodeB Endpoint
				_controlPointB.x = cos(angle)*radius + _endPointB.x;
				_controlPointB.y = -sin(angle)*radius + _endPointB.y;
			}
			else
			{
				// NodeB Endpoint
				_controlPointB.x = _endPointB.x - cos(angle)*radius;
				_controlPointB.y = sin(angle)*radius + _endPointB.y;

				//NodeA Endpoint
				_controlPointA.x = cos(angle)*radius + _endPointA.x;
				_controlPointA.y = -sin(angle)*radius + _endPointA.y;
			}


			//_controlPointA.x = _midpoint.x;
			//_controlPointB.x = _midpoint.x;

			//_controlPointA.y = _midpoint.y;
			//_controlPointB.y = _midpoint.y;

			_bDrawLinear = true;

			RefreshGeometry();
		}
		/**
		*
		*
		*
		*/
		void Link::SetOperationalState(LinkOperationalState OperationalState)
		{
			_linkOperationalState = OperationalState;
		}
		/**
		*
		*
		*
		*/
		void Link::SetDrawSuspendedStatus(bool bDrawSuspended)
		{ 
			_bDrawSuspended = bDrawSuspended; 
			RefreshGeometry();
		}
		/**
		*
		*
		*/
		void Link::SetDrawControlPoints(bool bDrawControlPoints)
		{ 
			_bDrawControlPoints = bDrawControlPoints;
		}
		/**
		*
		*
		*
		*/
		void Link::SetLineColor(D2D1_COLOR_F linkUserColorNormalState) 
		{ 
			_linkUserColorNormalState = linkUserColorNormalState; 
		}
		/**
		*
		*
		*
		*/
		void Link::SetLineWidth(float fWidth) 
		{ 
			_fWidth = fWidth + 0.5f; 
		}
		/**
		*
		*
		*
		*/
		void Link::SetSelectedState(bool bSelected) 
		{ 
			_bSelected = bSelected; 
		}
		/**
		*
		*
		*
		*/
		D2D1_POINT_2F Link::GetBezierMidPoint()
		{
			D2D1_POINT_2F p;
			Bezier(p, _endPointA, _controlPointA, _controlPointB, _endPointB, 0.5f);
			return p;
		}
		/**
		* GetLinkPathPoints
		*
		* Public function
		*/
		void Link::GetLinkPathPoints(int spatialGridSize, LINKPATHPOINTS& points)
		{
			// only one type of line currently....
			GetBezierPathPoints(spatialGridSize, points);
			return;
		}
		/**
		* GetPathPoints (private)
		*
		* Returns a sample of points on the Bézier line/path segment given
		*/
		void Link::GetBezierPathPoints(int spatialGridSize, LINKPATHPOINTS& points)
		{
			// calculate points in cubic Bézier curve, calculate cell from points and add cell to set
			for (int i = 0; i<_bezierSamplePoints; ++i)
			{
				D2D1_POINT_2F p;
				float t = static_cast<float>(i) / static_cast<float>(_bezierSamplePoints - 1);
				Bezier(p, _endPointA, _controlPointA, _controlPointB, _endPointB, t);
				points.insert(::std::make_pair(static_cast<int>(p.x / spatialGridSize), static_cast<int>(p.y / spatialGridSize)));
			}
			return;
		}
		/**
		* Lerp
		*
		* simple linear interpolation between two points
		*/
		void Link::Lerp(D2D1_POINT_2F& dest, const D2D1_POINT_2F& a, const D2D1_POINT_2F& b, const float t)
		{
			dest.x = a.x + (b.x - a.x)*t;
			dest.y = a.y + (b.y - a.y)*t;
		}
		/**
		* Bezier
		*
		*
		* evaluate a point on a cubic bezier-curve. t goes from 0 to 1.0
		*/
		void Link::Bezier(D2D1_POINT_2F &dest, const D2D1_POINT_2F& a, const D2D1_POINT_2F& b, const D2D1_POINT_2F& c, const D2D1_POINT_2F& d, const float t)
		{
			D2D1_POINT_2F ab, bc, cd, abbc, bccd;
			Lerp(ab, a, b, t);           // point between a and b (green)
			Lerp(bc, b, c, t);           // point between b and c (green)
			Lerp(cd, c, d, t);           // point between c and d (green)
			Lerp(abbc, ab, bc, t);       // point between ab and bc (blue)
			Lerp(bccd, bc, cd, t);       // point between bc and cd (blue)
			Lerp(dest, abbc, bccd, t);   // point on the bezier-curve (black)
		}
		// ns
	}
}

