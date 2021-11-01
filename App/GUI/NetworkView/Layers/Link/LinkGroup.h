#pragma once
#include "interfaces\objectdatabasedefines.h"
#include "Link.h"
#include <map>
#include <memory>
// Direct 2d
#include <d2d1.h>
#include <d2d1_1.h>
#include <D2d1_1helper.h>
#include <d2d1effects.h>
#include <d2d1effecthelpers.h>
#include <wrl.h>


namespace NM
{
	namespace ODB
	{
		class IObjectDatabase;
	}

	namespace NetGraph
	{
		class Link;
		//class LinkSpatialHash;
		class NetworkViewSpatialHash;
		class LinkDrawResources;

		class LinkGroup
		{
		public:
			// for map
			struct compareGuid
			{
				bool operator()(const ::NM::ODB::ODBUID& guid1, const::NM::ODB::ODBUID& guid2) const
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

			enum class LinkDrawState {Collapsed, Expanded};
			enum class LinkGroupDrawState { Normal, Stub };
			

			LinkGroup(::Microsoft::WRL::ComPtr<ID2D1Factory>, ::std::shared_ptr<NetworkViewSpatialHash>, ::NM::ODB::OBJECTUID nodeA, ::NM::ODB::OBJECTUID nodeB);
			~LinkGroup();

			::std::shared_ptr<Link> InsertLink(::NM::ODB::OBJECTUID);
			void DeleteLink(::NM::ODB::OBJECTUID);
			void RenderLinkGroup(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> &_pRenderTarget, ::NM::ODB::OBJECTUID activeGroupUID, ::std::shared_ptr<LinkDrawResources> pDrawResources);
			::std::shared_ptr<Link> GetLink(::NM::ODB::OBJECTUID);
			size_t GetLinkCount() { return _uidlinkmap.size(); };
			void GetEndPointObjectUIDs(::NM::ODB::OBJECTUID&, ::NM::ODB::OBJECTUID&);
			::NM::ODB::OBJECTUID GetCollapsedLinkUID();
			void SetLinkGroupExpanded(LinkDrawState drawState);
			LinkDrawState GetLinkGroupExpanded() { return _linkDrawState; };
			void SetNodePositions(D2D1_POINT_2F posA, D2D1_POINT_2F posB);
			void RefreshContent(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> &pRenderTarget);
			void SetLinkOperationalState(::NM::ODB::OBJECTUID, Link::LinkOperationalState);
			void DistributeLinks();
			::std::vector<::NM::ODB::OBJECTUID> GetGroupLinksUID();
			bool IsInGroup(::NM::ODB::OBJECTUID);
			void GetLinkGUIGroups(::NM::ODB::OBJECTUID& nodeAGroupUID, ::NM::ODB::OBJECTUID& nodeBGroupUID);

		private:

			//::std::shared_ptr<LinkSpatialHash>						_spatialHash;
			::std::shared_ptr<NetworkViewSpatialHash>						_spatialHash;
			::NM::ODB::OBJECTUID _nodeA;
			::NM::ODB::OBJECTUID _nodeB;
			::NM::ODB::OBJECTUID _nodeAGroupUID;
			::NM::ODB::OBJECTUID _nodeBGroupUID;
			::NM::ODB::OBJECTUID _parentGroup;
			::NM::ODB::OBJECTUID _childGroup;
			::Microsoft::WRL::ComPtr<ID2D1Factory>					_pD2DFactory;
			::std::unique_ptr<::NM::ODB::IObjectDatabase>			_odb;					// interface to the database
			LinkDrawState											_linkDrawState;
			LinkGroupDrawState										_linkGroupDrawState;

			::std::shared_ptr<Link> pLinkCollapsed;
			typedef ::std::map<::NM::ODB::OBJECTUID, ::std::shared_ptr<Link>, compareGuid> UIDLINKMAP;
			UIDLINKMAP _uidlinkmap;

			void CreateCollapsedLink();
			//void UpdateSpatialHash(::std::shared_ptr<Link>);
			void RefreshGroupOperationalState();
			void CalculateEndPoints(D2D1_POINT_2F& endpointsNodeA, D2D1_POINT_2F& endpointsNodeB);
			


			//D2D1_POINT_2F testLinePtA;
			//D2D1_POINT_2F testLinePtB;

		};

		// ns
	}
}

