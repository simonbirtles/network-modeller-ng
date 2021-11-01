#include "stdafx.h"
#include "INetworkView.h"
#include "INetworkViewService.h"


namespace NM
{
	namespace NetGraph
	{

		/**
		*
		*
		*
		*/
		INetworkView::INetworkView(INetworkViewService* pNetworkViewService):
			_pNetworkViewService(pNetworkViewService)
		{
		}
		/**
		*
		*
		*
		*/
		INetworkView::~INetworkView()
		{
		}
		/**
		*
		*
		*
		*/
		void INetworkView::SetSpotlightOn(::std::vector<::NM::ODB::OBJECTUID>& objectList)
		{
			assert(_pNetworkViewService);
			_pNetworkViewService->SetSpotlightOn(objectList);
		}
		/**
		*
		*
		*
		*/
		void INetworkView::SetSpotlightOff()
		{
			assert(_pNetworkViewService);
			_pNetworkViewService->SetSpotlightOff();
		}
		/**
		*
		*
		*
		*/
		bool INetworkView::SetGroupView(::NM::ODB::OBJECTUID groupUID)
		{
			assert(_pNetworkViewService);
			return _pNetworkViewService->SetGroupView(groupUID);
		}
		// ns
	}

}