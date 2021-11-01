#include "stdafx.h"
#include "LinkGroup.h"
#include "link.h"
#include "LinkDrawResources.h"
//#include "LinkSpatialHash.h"
#include "NetworkViewSpatialHash.h"
#include "IServiceRegistry.h"							// registry interface
#include "Interfaces\IObjectDatabase.h"					// for client interface to core database 

using ::NM::ODB::OBJECTUID;
using ::NM::ODB::UPVALUE;
using ::NM::ODB::INVALID_OBJECT_UID;
using ::NM::ODB::real_int;
using ::NM::ODB::real_uid;
using ::NM::ODB::real_string;
using ::NM::ODB::real_bool;

extern ::NM::Registry::IServiceRegistry* reg;

namespace NM
{
	namespace NetGraph
	{
		/**
		*
		*
		*
		*/
		LinkGroup::LinkGroup(::Microsoft::WRL::ComPtr<ID2D1Factory> pD2DFactory, ::std::shared_ptr<NetworkViewSpatialHash> pSpatialHash, ::NM::ODB::OBJECTUID nodeA, ::NM::ODB::OBJECTUID nodeB) :
			_pD2DFactory(pD2DFactory),
			_linkDrawState(LinkDrawState::Collapsed),
			_linkGroupDrawState(LinkGroupDrawState::Normal),
			_nodeA(::NM::ODB::INVALID_OBJECT_UID),
			_nodeB(::NM::ODB::INVALID_OBJECT_UID),
			_nodeAGroupUID(::NM::ODB::INVALID_OBJECT_UID),
			_nodeBGroupUID(::NM::ODB::INVALID_OBJECT_UID),
			_parentGroup(::NM::ODB::INVALID_OBJECT_UID),
			_childGroup(::NM::ODB::INVALID_OBJECT_UID),
			_spatialHash(pSpatialHash)
			//testLinePtA({ 0, 0 }),
			//testLinePtB({ 0, 0 })


		{
			if(!_pD2DFactory)
				throw ::std::runtime_error("NULL ID2D1Factory, Cannot Continue.");

			// if the global registry is not available we are in trouble
			if (!reg)
				throw ::std::runtime_error("Application Registry Not Available, Cannot Continue.");

			// Get The Database Handle and save
			_odb.reset(reinterpret_cast<NM::ODB::IObjectDatabase*>(reg->GetClientInterface(L"ODB")));
			if (!_odb)
				throw ::std::runtime_error("Database Not Available, Cannot Continue.");

			_nodeA = nodeA;
			_nodeB = nodeB;
			// get the endpoint groups IDs
			_nodeAGroupUID = _odb->GetValue(_nodeA, L"group")->Get<::NM::ODB::ODBUID>();
			_nodeBGroupUID = _odb->GetValue(_nodeB, L"group")->Get<::NM::ODB::ODBUID>();

		}
		/**
		*
		*
		*
		*/
		LinkGroup::~LinkGroup()
		{
			// delete all links in this group
			UIDLINKMAP::iterator it = _uidlinkmap.begin();
			while (it != _uidlinkmap.end())
			{
				_spatialHash->DeleteSpatialHashNode(it->first);
				it->second.reset();
				//_uidlinkmap.erase(it);
				++it;
			}
		}
		/**
		*
		*
		*
		*/
		::std::shared_ptr<Link> LinkGroup::InsertLink(::NM::ODB::OBJECTUID objectUID)
		{
			// link specific
			real_int controlpointAxpos = _odb->GetValue(objectUID, L"controlpointAxpos")->Get<::NM::ODB::ODBInt>();
			real_int controlpointAypos = _odb->GetValue(objectUID, L"controlpointAypos")->Get<::NM::ODB::ODBInt>();
			real_int controlpointBxpos = _odb->GetValue(objectUID, L"controlpointBxpos")->Get<::NM::ODB::ODBInt>();
			real_int controlpointBypos = _odb->GetValue(objectUID, L"controlpointBypos")->Get<::NM::ODB::ODBInt>();
			real_bool linkOperational = _odb->GetValue(objectUID, L"inservice")->Get<::NM::ODB::ODBBool>();
			real_int linkWidth = _odb->GetValue(objectUID, L"linewidth")->Get<::NM::ODB::ODBInt>();
			::NM::ODB::real_color linkColor = _odb->GetValue(objectUID, L"color")->Get<::NM::ODB::ODBColor>();


			D2D1_POINT_2F endpointsNodeA = { 0,0 };
			D2D1_POINT_2F endpointsNodeB = { 0,0 };
			CalculateEndPoints(endpointsNodeA, endpointsNodeB);
			
			// create new link instance for this group
			::std::shared_ptr<Link> pLink = ::std::make_shared<Link>(_pD2DFactory, objectUID, false);
			// set link attributes
			pLink->SetEndPoints(endpointsNodeA, endpointsNodeB);
			pLink->SetControlPoints(
				::D2D1::Point2F((float)controlpointAxpos, (float)controlpointAypos),	// control point 1
				::D2D1::Point2F((float)controlpointBxpos, (float)controlpointBypos)		// control point 2
				);
			pLink->SetOperationalState( (linkOperational ? Link::LinkOperationalState::Up : Link::LinkOperationalState::Down));
			pLink->SetLineWidth((float)linkWidth);
			pLink->SetLineColor(
				::D2D1::ColorF(
					(UINT32)(
						linkColor.GetBlue() |
						linkColor.GetGreen() << 8 |
						linkColor.GetRed() << 16
						), 1.0f
					));

			// local data cache updates
			_uidlinkmap[objectUID] = pLink;


			if (_uidlinkmap.size() > 1)
			{
				if(!pLinkCollapsed)
					CreateCollapsedLink();

				SetLinkGroupExpanded(_linkDrawState);
			}
			else
			{
				Link::LINKPATHPOINTS points;
				pLink->GetLinkPathPoints(_spatialHash->GetSpatialGridSize(), points);
				_spatialHash->InsertSpatialHashNode(pLink->GetLinkUID(), points );
			}

			RefreshGroupOperationalState();

			return pLink;
		}
		/**
		* CalculateEndPoints
		*
		* Calculates endpoints of line based upon the nodes assigned groups and 
		* the parent/child relationship, if any.
		*/
		void LinkGroup::CalculateEndPoints(D2D1_POINT_2F& endpointsNodeA, D2D1_POINT_2F& endpointsNodeB)
		{
			// get link endpoints
			real_int xposNodeA = _odb->GetValue(_nodeA, L"xpos")->Get<::NM::ODB::ODBInt>();
			real_int yposNodeA = _odb->GetValue(_nodeA, L"ypos")->Get<::NM::ODB::ODBInt>();

			real_int xposNodeB = _odb->GetValue(_nodeB, L"xpos")->Get<::NM::ODB::ODBInt>();
			real_int yposNodeB = _odb->GetValue(_nodeB, L"ypos")->Get<::NM::ODB::ODBInt>();

			// set up defaults (same group)
			endpointsNodeA.x = static_cast<float>(xposNodeA);
			endpointsNodeA.y = static_cast<float>(yposNodeA);
			endpointsNodeB.x = static_cast<float>(xposNodeB);
			endpointsNodeB.y = static_cast<float>(yposNodeB);

			// nodes on same level, use actual x,y points for each
			if (_nodeAGroupUID == _nodeBGroupUID)
			{
				return;
			}

			// nodes on different levels, find the parent and child.
			::NM::ODB::OBJECTUID rootGroup = ::NM::ODB::INVALID_OBJECT_UID;
			::std::vector<::NM::ODB::OBJECTUID> nodeAGroupList;
			::std::vector<::NM::ODB::OBJECTUID> nodeBGroupList;

			// get hierarcy of nodeA's groups back to root
			::NM::ODB::OBJECTUID currentGroup = _nodeAGroupUID;
			nodeAGroupList.push_back(currentGroup);
			while (currentGroup != rootGroup)
			{
				currentGroup = _odb->GetValue(currentGroup, L"group")->Get<::NM::ODB::ODBUID>();
				nodeAGroupList.push_back(currentGroup);
			}

			// get hierarcy of nodeB's groups back to root
			currentGroup = _nodeBGroupUID;
			nodeBGroupList.push_back(currentGroup);
			while (currentGroup != rootGroup)
			{
				currentGroup = _odb->GetValue(currentGroup, L"group")->Get<::NM::ODB::ODBUID>();
				nodeBGroupList.push_back(currentGroup);
			}


			// if... then NodeB is in the parent group, set the child endpoint (nodeA) to be the group node A
			if (nodeAGroupList.size() > nodeBGroupList.size())
			{
				real_int xposNodeA = _odb->GetValue(_nodeAGroupUID, L"xpos")->Get<::NM::ODB::ODBInt>();
				real_int yposNodeA = _odb->GetValue(_nodeAGroupUID, L"ypos")->Get<::NM::ODB::ODBInt>();
				endpointsNodeA.x = static_cast<float>(xposNodeA);
				endpointsNodeA.y = static_cast<float>(yposNodeA);
				_parentGroup = _nodeBGroupUID;
				_childGroup = _nodeAGroupUID;
			}
			// else if... then NodeA is in the parent group, set the child endpoint (nodeB) to be the group node 
			else if (nodeBGroupList.size() > nodeAGroupList.size())
			{
				real_int xposNodeB = _odb->GetValue(_nodeBGroupUID, L"xpos")->Get<::NM::ODB::ODBInt>();
				real_int yposNodeB = _odb->GetValue(_nodeBGroupUID, L"ypos")->Get<::NM::ODB::ODBInt>();
				endpointsNodeB.x = static_cast<float>(xposNodeB);
				endpointsNodeB.y = static_cast<float>(yposNodeB);
				_parentGroup = _nodeAGroupUID;
				_childGroup = _nodeBGroupUID;
			}
			// else.. both nodes at same group level but in different groups which is not allowed
			else
			{
				throw ::std::runtime_error("LinkGroup::CalculateEndPoints: Both nodes at same group level but in different groups which is not allowed");
			}


			return;
		}
		/**
		*
		*
		*
		*/
		void LinkGroup::CreateCollapsedLink()
		{
			// check not already created.
			if (pLinkCollapsed) return;
			
			if (_uidlinkmap.size() < 2) return;

			real_int xposNodeA = _odb->GetValue(_nodeA, L"xpos")->Get<::NM::ODB::ODBInt>();
			real_int yposNodeA = _odb->GetValue(_nodeA, L"ypos")->Get<::NM::ODB::ODBInt>();

			real_int xposNodeB = _odb->GetValue(_nodeB, L"xpos")->Get<::NM::ODB::ODBInt>();
			real_int yposNodeB = _odb->GetValue(_nodeB, L"ypos")->Get<::NM::ODB::ODBInt>();

			// find the midpoint between the verticies
			float midpoint_x = (static_cast<float>((xposNodeA + xposNodeB)) / 2);
			float midpoint_y = (static_cast<float>((yposNodeA + yposNodeB)) / 2);

			::NM::ODB::OBJECTUID internalNodeUid = ::NM::ODB::INVALID_OBJECT_UID;
			UuidCreate(&internalNodeUid);
			pLinkCollapsed = ::std::make_shared<Link>(_pD2DFactory, internalNodeUid, true);

			pLinkCollapsed->SetEndPoints(::D2D1::Point2F((float)xposNodeA, (float)yposNodeA), ::D2D1::Point2F((float)xposNodeB, (float)yposNodeB));

			pLinkCollapsed->SetControlPoints(
				::D2D1::Point2F((float)midpoint_x, (float)midpoint_y),	// control point 1
				::D2D1::Point2F((float)midpoint_x, (float)midpoint_y)		// control point 2
				);		

			RefreshGroupOperationalState();

			//pLinkCollapsed->SetOperationalState(true);		// need to change to enum to add a unknown/mixed state incase mix of operational states of links aggregated

			return;
		}
		/**
		*
		*
		*
		*/
		//void LinkGroup::UpdateSpatialHash(::std::shared_ptr<Link> pLink)
		//{
		//	/*if (_linkDrawState == LinkDrawState::Expanded)
		//		_spatialHash->InsertSpatialHashNode(pLink);*/

		//	return;
		//}
		/**
		*
		*
		*
		*/
		void LinkGroup::DeleteLink(::NM::ODB::OBJECTUID objectUID)
		{
			UIDLINKMAP::iterator it = _uidlinkmap.find(objectUID);
			if (it != _uidlinkmap.end())
			{
				//_spatialHash->DeleteSpatialHashNode(it->first);
				it->second.reset();  
				_uidlinkmap.erase(it);
			}
			RefreshGroupOperationalState();
			return;
		}
		/**
		* RenderLinkGroup
		*
		* Requires/Assumes rendertarget->Draw to be called already
		*/
		void LinkGroup::RenderLinkGroup(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> &_pRenderTarget, ::NM::ODB::OBJECTUID activeGroupUID, ::std::shared_ptr<LinkDrawResources> pDrawResources)
		{
			if (!IsInGroup(activeGroupUID))
				return;

			// if childGroup is invalid, then same level, or if currently in parentGroup, allow normal draw
			if ((_childGroup == ::NM::ODB::INVALID_OBJECT_UID) || (_parentGroup == activeGroupUID))
			{
				if (_linkGroupDrawState == LinkGroupDrawState::Stub)
				{
					_linkGroupDrawState = LinkGroupDrawState::Normal;
					/**
					* 1. If current state is LinkGroupDrawState::Stub, change to LinkGroupDrawState::Normal
					* 2. Based on LinkDrawState (or pre Stub LinkdrawState), UpdateSpatialHash for Collapsed or Expanded nodes
					* 3.
					*/
				}

				// check enum status _linkDrawState;
				if ((_linkDrawState == LinkDrawState::Collapsed) && _uidlinkmap.size() > 1)
				{
					if (!pLinkCollapsed)
						CreateCollapsedLink();

					pLinkCollapsed->RenderLink(_pRenderTarget, pDrawResources);
				}
				else /*LinkDrawState::Expanded*/
				{
					UIDLINKMAP::iterator it = _uidlinkmap.begin();
					while (it != _uidlinkmap.end())
					{
						it->second->RenderLink(_pRenderTarget, pDrawResources);
						++it;
					}
				}
			}
			// else we should be in the child group where we have connected nodes with a parent/child group relationship
			else
			{
				assert(_childGroup == activeGroupUID);

				// special draw as we draw from child node in this group 
				// draw a short line from child node to space with some kind of endpoint
				// need to think about link selection etc etc.. 
				if (_linkGroupDrawState == LinkGroupDrawState::Normal)
				{
					_linkGroupDrawState = LinkGroupDrawState::Stub;

					/*
					* 1. Set Mode To LinkGroupDrawState::Stub
					* 2. Create (if not exist) new links (copy from existing)
					* 3. Update copied Links endpoints and/or other info
					* 4. Update SpatialHash
					* 4. Draw Links, need to include node ellipse and text at stub end of link, poss include in Link class when calling RenderStubLink
					*
					* NB: Prevent changing LinkDrawState while in LinkGroupDrawState::Stub or do we save last LinkDrawState before moving to Stub ?
					*     By copying the links, when a link is selected, its just the same. need to be careful of DB Updates as we need to update both real and copied
					*
					*
					*/
				}

				OutputDebugString(L"\nDraw Child->Parent(Space) Links");
				// always draw expanded
				//UIDLINKMAP::iterator it = _uidlinkmap.begin();
				//while (it != _uidlinkmap.end())
				//{
				//	it->second->RenderStubLink(_pRenderTarget);
				//	++it;
				//}


			}


			//::ID2D1SolidColorBrush* brush;
			//_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 1.0f, 1.0f, 1.0f), &brush);
			//_pRenderTarget->DrawLine(testLinePtA, testLinePtB, brush);

			return;
		}
		/**
		*
		*
		*
		*/
		void LinkGroup::RefreshContent(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> &pRenderTarget)
		{
			if(pLinkCollapsed)
				pLinkCollapsed->RefreshContent(pRenderTarget);
		
			UIDLINKMAP::iterator it = _uidlinkmap.begin();
			while (it != _uidlinkmap.end())
			{
				it->second->RefreshContent(pRenderTarget);
				++it;
			}
			return;
		}
		/**
		* GetLink
		*
		* Returns a shared ptr to the Link instance for the provided ObjectUID
		*/
		::std::shared_ptr<Link> LinkGroup::GetLink(::NM::ODB::OBJECTUID objectUID)
		{
			UIDLINKMAP::iterator it = _uidlinkmap.find(objectUID);
			if (it != _uidlinkmap.end())
			{
				return it->second;
			}

			if (objectUID == pLinkCollapsed->GetLinkUID())
				return pLinkCollapsed;

			return nullptr;
		}
		/**
		*
		*
		*
		*
		*/
		::std::vector<::NM::ODB::OBJECTUID> LinkGroup::GetGroupLinksUID()
		{
			::std::vector<::NM::ODB::OBJECTUID> linkIDs;
			UIDLINKMAP::iterator it = _uidlinkmap.begin();
			while (it != _uidlinkmap.end())
			{
				linkIDs.push_back(it->first);
				++it;
			}
			return linkIDs;
		}
		/**
		* IsInGroup
		*
		* Returns the Node ObjectUID that is in the currently active GUI Group, this 
		* is the node that will be drawn by the node layer
		*/
		bool LinkGroup::IsInGroup(::NM::ODB::OBJECTUID activeGroupUID)
		{
			if(activeGroupUID == _nodeAGroupUID)
			{
				return true;
			}

			if(activeGroupUID == _nodeBGroupUID)
			{
				return true;
			}

			return false;
		}
		/**
		*
		*
		*
		*/
		void LinkGroup::GetLinkGUIGroups(::NM::ODB::OBJECTUID& nodeAGroupUID, ::NM::ODB::OBJECTUID& nodeBGroupUID)
		{
			nodeAGroupUID = _nodeAGroupUID;
			nodeBGroupUID = _nodeBGroupUID;
			return;
		}
		/**
		*
		*
		*
		*/
		void LinkGroup::GetEndPointObjectUIDs(::NM::ODB::OBJECTUID& nodeA, ::NM::ODB::OBJECTUID& nodeB)
		{
			nodeA = _nodeA;
			nodeB = _nodeB;
			return;
		}
		/**
		*
		*
		*
		*/
		void LinkGroup::SetLinkGroupExpanded(LinkDrawState drawState)
		{
			if (!pLinkCollapsed) return;

			// if state change
			//if (_linkDrawState != drawState)
			{
				switch (drawState)
				{
				case LinkDrawState::Expanded:	/* new state requested*/
				{
					// remove collapsedlink spatial 
					_spatialHash->DeleteSpatialHashNode(pLinkCollapsed->GetLinkUID());

					// add individual links to spatial
					UIDLINKMAP::iterator it =  _uidlinkmap.begin();
					while (it != _uidlinkmap.end())
					{
						Link::LINKPATHPOINTS points;
						it->second->GetLinkPathPoints(_spatialHash->GetSpatialGridSize(), points);
						_spatialHash->InsertSpatialHashNode(it->second->GetLinkUID(), points);
						//_spatialHash->InsertSpatialHashNode(it->second);
						++it;
					}
				}
					break;

				case LinkDrawState::Collapsed: /* new state requested*/
				{
					// add collapsedlink spatial 
					Link::LINKPATHPOINTS points;
					pLinkCollapsed->GetLinkPathPoints(_spatialHash->GetSpatialGridSize(), points);
					_spatialHash->InsertSpatialHashNode(pLinkCollapsed->GetLinkUID(), points);
					//_spatialHash->InsertSpatialHashNode(pLinkCollapsed);

					// remove individual links to spatial
					UIDLINKMAP::iterator it = _uidlinkmap.begin();
					while (it != _uidlinkmap.end())
					{
						_spatialHash->DeleteSpatialHashNode(it->first);
						++it;
					}
				}
					break;

				default:
					break;
				}
				// change draw state indicator
				_linkDrawState = drawState;
			}
			
			return;
		}
		/**
		*
		*
		*
		*/
		::NM::ODB::OBJECTUID LinkGroup::GetCollapsedLinkUID()
		{
			if (pLinkCollapsed) return pLinkCollapsed->GetLinkUID();

			return ::NM::ODB::INVALID_OBJECT_UID;
		}
		/**
		*
		*
		* Sets all the links x,y pos in this group
		*/
		void LinkGroup::SetNodePositions(D2D1_POINT_2F posA, D2D1_POINT_2F posB)
		{
			CalculateEndPoints(posA, posB);

			UIDLINKMAP::iterator it = _uidlinkmap.begin();
			while (it != _uidlinkmap.end())
			{
				it->second->SetEndPoints(posA, posB);
				Link::LINKPATHPOINTS points;
				it->second->GetLinkPathPoints(_spatialHash->GetSpatialGridSize(), points);
				_spatialHash->UpdateSpatialHashNode(it->second->GetLinkUID(), points);

				++it;
			}

			if (pLinkCollapsed)
			{
				pLinkCollapsed->SetEndPoints(posA, posB);
				Link::LINKPATHPOINTS points;
				pLinkCollapsed->GetLinkPathPoints(_spatialHash->GetSpatialGridSize(), points);
				_spatialHash->UpdateSpatialHashNode(pLinkCollapsed->GetLinkUID(), points);
			}

			return;
		}
		/**
		* DistributeLinks
		*
		* distributes the links control points to draw with uniform spacing
		*/
		void LinkGroup::DistributeLinks()
		{
			size_t linkCount = GetLinkCount();
			if (linkCount == 0 || !pLinkCollapsed) return;

			D2D1_POINT_2F posNodeA;
			D2D1_POINT_2F posNodeB;

			posNodeA.x = static_cast<float>(_odb->GetValue(_nodeA, L"xpos")->Get<::NM::ODB::ODBInt>());
			posNodeA.y = static_cast<float>(_odb->GetValue(_nodeA, L"ypos")->Get<::NM::ODB::ODBInt>());

			posNodeB.x = static_cast<float>(_odb->GetValue(_nodeB, L"xpos")->Get<::NM::ODB::ODBInt>());
			posNodeB.y = static_cast<float>(_odb->GetValue(_nodeB, L"ypos")->Get<::NM::ODB::ODBInt>());

			// calc the midpoint between the verticies
			D2D1_POINT_2F midpoint = ::D2D1::Point2F(((posNodeA.x + posNodeB.x) / 2), ((posNodeA.y + posNodeB.y) / 2));


			// slope
			float m = static_cast<float>(
				(posNodeB.y - posNodeA.y) /
				((posNodeB.x - posNodeA.x) == 0 ? 0.0001f : (posNodeB.x - posNodeA.x)));

			// perpendicular slope
			m = -1 / m;

			// perpendicular constant
			float c = static_cast<float>(midpoint.y - (m * midpoint.x));

			// angle
			float angle = atan(m)*-1;

			// for each link calculate the new control point poistions ( y = m*x+c )
			float x = midpoint.x;
			float y = midpoint.y;
			float radius = 10.0f;
			UIDLINKMAP::iterator linkit = _uidlinkmap.begin();
			while (linkit != _uidlinkmap.end())
			{
				x = midpoint.x - cos(angle)*radius;
				y = sin(angle)*radius + midpoint.y;

				linkit->second->SetControlPoints(::D2D1::Point2F(x, y), ::D2D1::Point2F(x, y));
				Link::LINKPATHPOINTS points;
				linkit->second->GetLinkPathPoints(_spatialHash->GetSpatialGridSize(), points);
				_spatialHash->UpdateSpatialHashNode(linkit->second->GetLinkUID(), points);

				// save to DB - really need this in LinkLayer as LinkGroup should not really be saving values
				// but need an iterator in LinkGroup for objectuids.
				_odb->SetValue(linkit->second->GetLinkUID(), L"controlpointAxpos", real_int(static_cast<real_int>(static_cast<int>(x))));
				_odb->SetValue(linkit->second->GetLinkUID(), L"controlpointAypos", real_int(static_cast<real_int>(static_cast<int>(y))));

				_odb->SetValue(linkit->second->GetLinkUID(), L"controlpointBxpos", real_int(static_cast<real_int>(static_cast<int>(x))));
				_odb->SetValue(linkit->second->GetLinkUID(), L"controlpointBypos", real_int(static_cast<real_int>(static_cast<int>(y))));

				radius = -radius;
				if(radius > 0)
					radius += 20;


				++linkit;
			}

			//testLinePtA.x = 0;
			//testLinePtA.y = (m * testLinePtA.x) + c;

			//testLinePtB.x = 1000;
			//testLinePtB.y = (m * testLinePtB.x) + c;



			return;
		}
		/**
		*
		*
		*
		*/
		void LinkGroup::SetLinkOperationalState(::NM::ODB::OBJECTUID objectUID, Link::LinkOperationalState OperationalState)
		{
			// set link
			UIDLINKMAP::iterator linkit = _uidlinkmap.find(objectUID);
			if (linkit != _uidlinkmap.end())
			{			
				linkit->second->SetOperationalState(OperationalState);
			}

			// set group
			RefreshGroupOperationalState();
			return;
		}
		/**
		*
		*
		*
		*/
		void LinkGroup::RefreshGroupOperationalState()
		{
			if (!pLinkCollapsed) return;

			size_t opStateUp = 0;
			size_t opStateDown = 0;

			UIDLINKMAP::iterator linkit = _uidlinkmap.begin();
			while (linkit != _uidlinkmap.end())
			{
				 Link::LinkOperationalState opStateLink = linkit->second->GetOperationalState();
				 if (opStateLink == Link::LinkOperationalState::Up) opStateUp++;
				 if (opStateLink == Link::LinkOperationalState::Down) opStateDown++;
				 ++linkit;
			}

			if (opStateUp > 0 && opStateDown > 0)
				pLinkCollapsed->SetOperationalState(Link::LinkOperationalState::Unknown);

			if (opStateUp > 0 && opStateDown == 0)
				pLinkCollapsed->SetOperationalState(Link::LinkOperationalState::Up);

			if (opStateUp == 0 && opStateDown > 0)
				pLinkCollapsed->SetOperationalState(Link::LinkOperationalState::Down);

			return;
		}

		// ns
	}
}
