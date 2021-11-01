#include "stdafx.h"
#include "INetworkViewService.h"
#include "INetworkView.h"
#include "NetworkView.h"
#include "BackgroundLayer.h"
#include "NodeLayer.h"
#include "LinkLayer.h"
//#include "TextLayer.h" - used for original testing
#include "SpotlightLayer.h"

namespace NM 
{
	namespace NetGraph 
	{
		CNetworkView* INetworkViewService::_pNetworkView = nullptr;
		/**
		*
		*
		*
		*/
		INetworkViewService::INetworkViewService():
			_pSpotlight(nullptr)
		{
		}
		/**
		*
		*
		*
		*/
		INetworkViewService::~INetworkViewService()
		{
		}
		/**
		*
		*
		*
		*/
		bool INetworkViewService::StartRegService()
		{			
			// create instance (Background Layer)
			::NM::NetGraph::BackgroundLayer *bg = new ::NM::NetGraph::BackgroundLayer;
			_pNetworkView->CreateClient(bg);
							
			// create instance (Link Layer)
			::NM::NetGraph::LinkLayer* link = new ::NM::NetGraph::LinkLayer;
			_pNetworkView->CreateClient(link);
			
			// create instance (Node Layer)
			::NM::NetGraph::NodeLayer* node = new ::NM::NetGraph::NodeLayer;
			_pNetworkView->CreateClient(node);
						
			// create instance (Spotlight Layer)
			_pSpotlight = new ::NM::NetGraph::SpotLightLayer;
			_pNetworkView->CreateClient(_pSpotlight);

			//	::NM::NetGraph::TextLayer* text = new ::NM::NetGraph::TextLayer;
			//_pNetworkView->CreateClient(text);
			
			return true;
		}
		/**
		*
		*
		*
		*/
		bool INetworkViewService::StopRegService()
		{
			return true;
		}
		/**
		*
		*
		*
		*/
		void* INetworkViewService::GetClientInterface()
		{
			return new INetworkView(this);
		}
		/**
		*
		*
		*
		*/
		void INetworkViewService::SetSpotlightOn(::std::vector<::NM::ODB::OBJECTUID> &objectList)
		{
			_pSpotlight->SetSpotlightOn(objectList);
		}
		/**
		*
		*
		*
		*/
		void INetworkViewService::SetSpotlightOff()
		{
			_pSpotlight->SetSpotlightOff();
		}
		/**
		*
		*
		*
		*/
		bool INetworkViewService::SetGroupView(::NM::ODB::OBJECTUID groupUID)
		{
			return _pNetworkView->SetGroupView(groupUID);
		}

		// ns
	}
}
