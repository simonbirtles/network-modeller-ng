#pragma once
#include "BaseLayer.h"
#include "LinkDrawResources.h"
#include "Interfaces\ObjectDatabaseDefines.h"
#include "Interfaces\IDatabaseUpdate.h"					// for client interface 
#include "Interfaces\DatabaseObserver.h"					// base class for client listener (dervive from for update calls)
#include "IAdjacencyMatrix.h"
#include "SelectedObjectsClientObs.h"
#include "NetworkViewSpatialHash.h"
#include <unordered_map>
#include <tuple>
#include <memory>

namespace NM
{
	class ISelectedObjects;

	namespace ODB
	{
		class IObjectDatabase;
	}

	namespace DataServices
	{
		class IAdjacencyMatrix;
	}

	namespace NetGraph
	{
		class LinkSpatialHash;
		class Link;
		class LinkGroup;
		class LinkLayerState;

		class LinkLayer :
			public BaseLayer,
			public ::NM::ODB::CDatabaseObserver,
			public ::NM::SelectedObjectsClientObs
		{
		public:
			typedef ::NM::ODB::OBJECTUID LINKUID;
			typedef ::NM::ODB::OBJECTUID NODEUID;
			// for map
			struct compareGuid
			{
				bool operator()(const NODEUID& guid1, const NODEUID& guid2) const
				{
					if (guid1.Data1 != guid2.Data1) {
						return guid1.Data1 < guid2.Data1;
					}
					if (guid1.Data2 != guid2.Data2) {
						return guid1.Data2 < guid2.Data2;
					}
					if (guid1.Data3 != guid2.Data3) {
						return guid1.Data3 < guid2.Data3;
					}
					for (int i = 0;i<8;i++) {
						if (guid1.Data4[i] != guid2.Data4[i]) {
							return guid1.Data4[i] < guid2.Data4[i];
						}
					}
					return false;
				}
			};
			struct compareGuidPair
			{
				bool operator()(const ::std::pair<NODEUID, NODEUID>& guidPair1, const ::std::pair<NODEUID, NODEUID>& guidPair2) const
				{
					if (guidPair1.first.Data1 != guidPair2.first.Data1) {
						return guidPair1.first.Data1 < guidPair2.first.Data1;
					}

					if (guidPair1.first.Data2 != guidPair2.first.Data2) {
						return guidPair1.first.Data2 < guidPair2.first.Data2;
					}

					if (guidPair1.first.Data3 != guidPair2.first.Data3) {
						return guidPair1.first.Data3 < guidPair2.first.Data3;
					}

					for (int i = 0;i<8;i++) {
						if (guidPair1.first.Data4[i] != guidPair2.first.Data4[i]) {
							return guidPair1.first.Data4[i] < guidPair2.first.Data4[i];
						}
					}

					if (guidPair1.second.Data1 != guidPair2.second.Data1) {
						return guidPair1.second.Data1 < guidPair2.second.Data1;
					}

					if (guidPair1.second.Data2 != guidPair2.second.Data2) {
						return guidPair1.second.Data2 < guidPair2.second.Data2;
					}

					if (guidPair1.second.Data3 != guidPair2.second.Data3) {
						return guidPair1.second.Data3 < guidPair2.second.Data3;
					}

					for (int i = 0;i<8;i++) {
						if (guidPair1.second.Data4[i] != guidPair2.second.Data4[i]) {
							return guidPair1.second.Data4[i] < guidPair2.second.Data4[i];
						}
					}

					return false;
				}
			};

			LinkLayer();
			~LinkLayer();

			HRESULT OnRender(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> &pRenderTarget, bool bRedraw);
			LRESULT ProcessMessage(UINT, WPARAM, LPARAM);
			BOOL	ProcessCommandMessage(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
			bool ShouldRender() { return true; };
			bool Initialise();
			::std::wstring GetLayerText() { return L"Link Layer"; }

			// from DatabaseUpdateCache
			void DatabaseUpdate();
			// From SelectedObjectService
			void SelectedObjectsUpdate();
			void MultiSelectChange(bool bMultiSelect);

			::std::shared_ptr<NetworkViewSpatialHash> GetLinkSpatialHash() { return _spatialHash; };

			void	 EndCurrentState() { _linkLayerState = nullptr; };  // temp


		private:
			IDWriteFactory* pDWriteFactory_;
			::Microsoft::WRL::ComPtr<IDWriteTextFormat> pTextFormat_;
			const wchar_t* wszText_;
			UINT32 cTextLength_;
			::Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pTextWhiteBrush_;

			typedef ::std::shared_ptr<LinkGroup> PLINKGRP;
			typedef ::std::shared_ptr<Link> PLINK;

			::std::unique_ptr<::NM::DataServices::IAdjacencyMatrix> _adjm;
			::std::shared_ptr<LinkLayerState>						_linkLayerState;
			::std::shared_ptr<LinkDrawResources>					_linkDrawResources;
			::std::shared_ptr<NetworkViewSpatialHash>				_spatialHash;
			::std::unique_ptr<::NM::ODB::IObjectDatabase>			_odb;					// interface to the database
			::std::unique_ptr<::NM::ODB::IDatabaseUpdate>			_updateCache;			// interface to the db update cache
			::NM::ODB::IDatabaseUpdate::UpdateQueueHandle			_updateQ;				// our personal queue of updates to db we requested
			::std::unique_ptr<::NM::ISelectedObjects>				_selectedObjectsSvc;
			::Microsoft::WRL::ComPtr<ID2D1Factory>					_pD2DFactory;
			::std::vector<::NM::ODB::OBJECTUID>						_selectedObjects;
			::NM::ODB::real_uid										_currentLayerUID;
			bool													_bSupressSelectedObjectUpdates;

			


			void	 BuildLinkCache();

			
			/* local cache of links
			*
			* Cache OBJECTUID to LinkGroup
			* Cache NodeA,NodeB to LinkGroup
			* 
			*/		
			// function wrappers required for write operations on these maps
			typedef ::NM::ODB::OBJECTUID GROUPUID;
			typedef ::std::set<::std::shared_ptr<LinkGroup>> LINKGROUPS;
			typedef ::std::map<LINKUID, ::std::shared_ptr<LinkGroup>, compareGuid> LINKGRPMAP;
			typedef ::std::map<::std::pair<NODEUID, NODEUID>, ::std::shared_ptr<LinkGroup>, compareGuidPair> NODEGRPMAP;
			typedef ::std::map<GROUPUID, ::std::vector<::std::shared_ptr<LinkGroup>>, compareGuid> GUIGRPLNKGMAP;
			LINKGROUPS		_linkGroups;
			LINKGRPMAP		_linkGrpMap;
			NODEGRPMAP		_nodeGrpMap;
			GUIGRPLNKGMAP	_guiGroupLinkGrpMap;



			// function wrappers for above
			PLINK	 InsertLink(LINKUID, NODEUID, NODEUID);
			PLINK	 GetLink(LINKUID);///////////////////////////////////////////////////////////////////remove
			PLINKGRP GetLinkGroup(LINKUID);
			void	 DeleteLink(LINKUID);

			// windows message handlers
			LRESULT OnLeftButtonUp(POINT p);
			LRESULT OnRightButtonDown(POINT p);
			LRESULT OnRightButtonUp(POINT p);
			LRESULT OnLeftButtonDown(POINT p);
			LRESULT OnLeftButtonDblClick(POINT p);
			LRESULT OnContextMenu(POINT p);

			
			void EditLinkControlPoints(::NM::ODB::OBJECTUID);
			size_t GetSelectedLinkCount() { return _selectedObjects.size(); }
			void ClearSelectedObjects();
			void DoSelectObject(PLINK);			
			PLINK GetLinkFromPoint(POINT p);
			bool IsControlKeyPressed();
			bool IsShiftKeyPressed();
			bool IsLeftButtonPressed();

			

		};

		// ns
	}
}

