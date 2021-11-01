#pragma once
#include "LinkLayerState.h"
#include "LinkDrawResources.h"
#include <memory>


namespace NM
{
	namespace NetGraph
	{

		class Link;
		//class LinkSpatialHash;
		class NetworkViewSpatialHash;

		class LinkLayerEditControlPointsState :
			public LinkLayerState
		{
		public:
			LinkLayerEditControlPointsState(LinkLayer*);
			~LinkLayerEditControlPointsState();

			LRESULT ProcessMessage(UINT, WPARAM, LPARAM, LinkLayer*, ::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, bool& Redraw);
			void    OnRender(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget, ::std::shared_ptr<LinkDrawResources> pDrawResources);
			void    StartEditLinkControlPoints(::std::shared_ptr<Link> pLink);
			void    EndEditLinkControlPoints();

		private:
			::std::shared_ptr<Link>				_pLink;
			::std::shared_ptr<Link>				_pLinkEdit;
			bool								_bMouseButtonDown;
			LinkLayer*							_pLinkLayer;
			::std::shared_ptr<NetworkViewSpatialHash>	_pSpatialHash;
			size_t								_controlPoint;


			LRESULT OnLeftButtonDown(POINT p);
			LRESULT OnMouseMove(POINT p);
			LRESULT OnLeftButtonUp(POINT p);

		};

	}
}

