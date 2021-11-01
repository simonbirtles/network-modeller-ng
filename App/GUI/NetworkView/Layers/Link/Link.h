#pragma once
#include "interfaces\objectdatabasedefines.h"
// Direct 2d
#include <d2d1.h>
#include <d2d1_1.h>
#include <D2d1_1helper.h>
#include <d2d1effects.h>
#include <d2d1effecthelpers.h>
#include <wrl.h>
#include <set>

namespace NM
{
	namespace NetGraph
	{

		class LinkDrawResources;

		class Link
		{
		public:
			enum class LinkOperationalState { Up, Down, Unknown };

			typedef ::std::set<::std::pair<int, int>> LINKPATHPOINTS;


			Link(::Microsoft::WRL::ComPtr<ID2D1Factory>, ::NM::ODB::OBJECTUID, bool);
			~Link();

			void RenderLink(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> &pRenderTarget, ::std::shared_ptr<LinkDrawResources> pDrawResources);
			void RefreshContent(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> &pRenderTarget);

			::NM::ODB::OBJECTUID GetLinkUID() {	return _objectUID;};
			void GetLinkPathPoints(int spatialGridSize, LINKPATHPOINTS& points);
			void SetEndPoints(D2D1_POINT_2F &pointA, D2D1_POINT_2F &pointB);
			void GetEndPoints(D2D1_POINT_2F &pointA, D2D1_POINT_2F &pointB);
			void SetControlPoints(D2D1_POINT_2F &controlPointA, D2D1_POINT_2F &controlPointB);
			void GetControlPoints(D2D1_POINT_2F &pointA, D2D1_POINT_2F &pointB);
			void SetOperationalState(LinkOperationalState OperationalState);
			LinkOperationalState GetOperationalState() { return _linkOperationalState; };			
			bool IsPointOnLine(D2D1_POINT_2F &point);
			D2D1_POINT_2F GetBezierMidPoint();
			D2D1_POINT_2F GetLineMidPoint() { return _midpoint;  };
			size_t IsPointOnControlPoint(D2D1_POINT_2F &point);
			void SetDrawSuspendedStatus(bool bDrawSuspended);
			void SetDrawControlPoints(bool bDrawControlPoints);
			void ResetControlPoints();
			void SetSelectedState(bool bSelected);
			bool IsInternalLink() { return _bInternalLink; };
			void SetLineWidth(float fWidth);
			void SetLineColor(D2D1_COLOR_F linkUserColorNormalState);
			


		private:
			bool					_bInternalLink;
			bool					_bDrawLinear;
			bool					_bDrawControlPoints;
			bool					_bDrawSuspended;
			bool					_bSelected;
			float					_fWidth;					// add this width to link specified width
			::NM::ODB::OBJECTUID	_objectUID;
			D2D1_POINT_2F			_endPointA;
			D2D1_POINT_2F			_endPointB;
			D2D1_POINT_2F			_midpoint;
			D2D1_POINT_2F			_controlPointA;
			D2D1_POINT_2F			_controlPointB;
			LinkOperationalState	_linkOperationalState;
			D2D1_COLOR_F			_linkUserColorNormalState;
			int						_bezierSamplePoints;

			::Microsoft::WRL::ComPtr<ID2D1Factory>		_pD2DFactory;
			::Microsoft::WRL::ComPtr<ID2D1PathGeometry> _pPathGeometry;

			::Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>	_nodebrushNormal;
			////::Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>	_nodebrushRed;
			////::Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>	_nodebrushControlPtsLine;
			////::Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>	_nodebrushSelected;
			////::Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>	_nodebrushStateDown;
			////::Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>	_nodebrushStateUnknown;

			void GetBezierPathPoints(int spatialGridSize, LINKPATHPOINTS& points);
			void Lerp(D2D1_POINT_2F& dest, const D2D1_POINT_2F& a, const D2D1_POINT_2F& b, const float t);
			void Bezier(D2D1_POINT_2F &dest, const D2D1_POINT_2F& a, const D2D1_POINT_2F& b, const D2D1_POINT_2F& c, const D2D1_POINT_2F& d, const float t);

			void RefreshGeometry();

		};

		// ns

	}
}

