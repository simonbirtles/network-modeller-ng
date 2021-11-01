#include "stdafx.h"
#include "LinkLayer.h"
#include "Link.h"
#include "LinkGroup.h"
//#include "LinkSpatialHash.h"
#include "NetworkViewSpatialHash.h"
#include "LinkLayerState.h"
#include "LinkLayerEditControlPointsState.h"
#include "IServiceRegistry.h"							// registry interface
#include "ISelectedObjects.h"							// holds references to currently selected objects application wide
#include "Interfaces\IObjectDatabase.h"					// for client interface to core database 
#include "Interfaces\IAttribute.h"						// interface to an attribute copy
#include "Interfaces\IDatabaseUpdate.h"					// for client interface for core database update notifications
#include "IAdjacencyMatrix.h"

extern NM::Registry::IServiceRegistry* reg;

using ::NM::ODB::OBJECTUID;
using ::NM::ODB::UPVALUE;
using ::NM::ODB::INVALID_OBJECT_UID;
using ::NM::ODB::real_int;
using ::NM::ODB::real_uid;
using ::NM::ODB::real_string;
using ::NM::ODB::real_bool;

namespace NM
{
	namespace NetGraph
	{
		/**
		*
		*
		*
		*/
		LinkLayer::LinkLayer() :
			_linkLayerState(nullptr),
			_currentLayerUID(::NM::ODB::INVALID_OBJECT_UID),
			_bSupressSelectedObjectUpdates(false)
		{
			
		}
		/**
		*
		*
		*
		*/
		LinkLayer::~LinkLayer()
		{
			//pTextWhiteBrush_->Release();
		}
		/**
		*
		*
		*
		*/
		bool LinkLayer::Initialise()
		{
			OutputDebugString(L"\nLinkLayer::Initialise");

			_spatialHash = GetSpatialHashPtr();
			if (!_spatialHash)
				throw ::std::runtime_error("Spatial Hash Service Not Available");

			// if the global registry is not available we are in trouble
			if (!reg)
				throw ::std::runtime_error("Application Registry Not Available, Cannot Continue.");

			// Get The Database Handle and save
			_odb.reset(reinterpret_cast<NM::ODB::IObjectDatabase*>(reg->GetClientInterface(L"ODB")));
			if (!_odb)
				throw ::std::runtime_error("Database Not Available, Cannot Continue.");

			// Get the handle to the database update cache
			_updateCache.reset(reinterpret_cast<::NM::ODB::IDatabaseUpdate*>(reg->GetClientInterface(L"ObjectUpdateCache")));
			if (!_updateCache)
				throw ::std::runtime_error("Database UpdateCache Not Available, Cannot Continue.");

			_selectedObjectsSvc.reset((reinterpret_cast<NM::ISelectedObjects*>(reg->GetClientInterface(L"SELECTEDOBJECTS"))));
			if (!_selectedObjectsSvc)
				throw ::std::runtime_error("Selected Objects Service Not Available, Cannot Continue.");

			_adjm.reset(static_cast<::NM::DataServices::IAdjacencyMatrix*>(reg->GetClientInterface(L"ADJMATRIX")));
			if (!_adjm)
				throw ::std::runtime_error("Adj. Matrix Service Not Available, Cannot Continue.");
								
			
			// get the current render target, we dont save it 
			// as it can change at anytime and become invalid
			::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget>	pRenderTarget;
			GetRenderTarget(pRenderTarget);
			assert(pRenderTarget);
			// get factory for links to create pathgeometries
			pRenderTarget->GetFactory(_pD2DFactory.GetAddressOf());
			assert(_pD2DFactory.Get());

			// create shared resources
			_linkDrawResources.reset(new LinkDrawResources);
			_linkDrawResources->RefreshContent(pRenderTarget);
			
			// Cache Edges and build spatial index
			BuildLinkCache();

			_selectedObjectsSvc->RegisterClientObserver(this);

			// register for specific database updates
			::std::vector<::std::wstring> attr_list;
			attr_list.push_back(L"xpos");
			attr_list.push_back(L"ypos");
			_updateQ = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Vertex, attr_list);
			_updateQ = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Group, attr_list);

			attr_list.clear();
			attr_list.push_back(L"inservice");
			attr_list.push_back(L"color");
			attr_list.push_back(L"linewidth");
			attr_list.push_back(L"create");
			attr_list.push_back(L"delete");
			_updateQ = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Edge, attr_list);

	

#pragma region LayerText
			// create direct write factory
			HRESULT hr = DWriteCreateFactory(
				DWRITE_FACTORY_TYPE_SHARED,
				__uuidof(IDWriteFactory),
				reinterpret_cast<IUnknown**>(&pDWriteFactory_)
				);

			wszText_ = L"Link Layer";
			cTextLength_ = (UINT32)wcslen(wszText_);
			pTextFormat_.Reset();
			if (SUCCEEDED(hr))
			{
				hr = pDWriteFactory_->CreateTextFormat(
					L"Gabriola",                // Font family name.
					NULL,                       // Font collection (NULL sets it to use the system font collection).
					DWRITE_FONT_WEIGHT_REGULAR,
					DWRITE_FONT_STYLE_NORMAL,
					DWRITE_FONT_STRETCH_NORMAL,
					36.0f,
					L"en-us",
					&pTextFormat_
					);
			}

			// Center align (horizontally) the text.
			if (SUCCEEDED(hr))
			{
				hr = pTextFormat_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
			}

			if (SUCCEEDED(hr))
			{
				hr = pTextFormat_->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
			}

#pragma endregion LayerText

			return true;
		}
		/**
		* InsertLink
		*
		* In: LinkUId, NodeA-UID, NodeB-UID
		* Out: Returns a ptr to the created link
		*
		* 1. Creates a LinkGroup if one does not already exist and adds to the local lookup NodeA,NodeB->LinkGroup
		* 2. Creates the Link and adds it to the LinkGroup and updates LinkUID->LinkGroup lookup
		*/
		LinkLayer::PLINK LinkLayer::InsertLink(LINKUID linkID, NODEUID nodeAID, NODEUID nodeBID)
		{
			PLINKGRP linkGroup;
			NODEGRPMAP::iterator nodeit = _nodeGrpMap.find(::std::make_pair(nodeAID, nodeBID));
			if (nodeit == _nodeGrpMap.end())
			{
				// create new link group
				linkGroup = ::std::make_shared<LinkGroup>(_pD2DFactory, _spatialHash, nodeAID, nodeBID);
				// add to local caches
				_linkGroups.insert(linkGroup);
				_nodeGrpMap[::std::make_pair(nodeAID, nodeBID)] = linkGroup;

				// cache guigroup - >nodeA
				::NM::ODB::OBJECTUID NodeAGroup = _odb->GetValue(nodeAID, L"group")->Get<::NM::ODB::ODBUID>();
				GUIGRPLNKGMAP::iterator ait = _guiGroupLinkGrpMap.find(NodeAGroup);
				if (ait != _guiGroupLinkGrpMap.end())
				{
					ait->second.push_back(linkGroup);
				}
				else
				{
					::std::vector<::std::shared_ptr<LinkGroup>> v;
					v.push_back(linkGroup);
					_guiGroupLinkGrpMap[NodeAGroup] = v;
				}

				// cache guigroup - >nodeB
				::NM::ODB::OBJECTUID NodeBGroup = _odb->GetValue(nodeBID, L"group")->Get<::NM::ODB::ODBUID>();
				GUIGRPLNKGMAP::iterator bit = _guiGroupLinkGrpMap.find(NodeBGroup);
				if (bit != _guiGroupLinkGrpMap.end())
				{
					bit->second.push_back(linkGroup);
				}
				else
				{
					::std::vector<::std::shared_ptr<LinkGroup>> v;
					v.push_back(linkGroup);
					_guiGroupLinkGrpMap[NodeBGroup] = v;
				}


			}
			else
			{
				linkGroup = nodeit->second;
			}
			
			// create new link in LinkGroup
			PLINK pLink = linkGroup->InsertLink(linkID);
			if (pLink)
			{
				// add reference from Link UID to its Parent LinkGroup instance.
				_linkGrpMap[linkID] = linkGroup;

				// add group collapsed linkuid
				if (linkGroup->GetLinkCount() > 1)
				{
					::NM::ODB::OBJECTUID linkGroupCollapsedUID = linkGroup->GetCollapsedLinkUID();
					assert(linkGroupCollapsedUID != ::NM::ODB::INVALID_OBJECT_UID);
					_linkGrpMap[linkGroupCollapsedUID] = linkGroup;

				}
			}
			
			return pLink;
		}
		/**
		* GetLink
		*
		* Get the Link class instance for the given ObjectUID
		*/
		LinkLayer::PLINK LinkLayer::GetLink(LINKUID objectUID)
		{
			PLINK pLink = nullptr;

			// 1. Get Link Group
			LinkLayer::PLINKGRP grp = GetLinkGroup(objectUID);
			if (!grp) return nullptr;

			// 2. Ask LinkGroup for Link*
			pLink = grp->GetLink(objectUID);
			
			return pLink;
		}
		/**
		*
		*
		*
		*/
		LinkLayer::PLINKGRP LinkLayer::GetLinkGroup(LINKUID objectUID)
		{
			LINKGRPMAP::iterator grpit = _linkGrpMap.find(objectUID);
			if (grpit == _linkGrpMap.end())
				return nullptr;

			return grpit->second;
		}
		/**
		*
		*
		*
		*/
		void LinkLayer::DeleteLink(LINKUID objectUID)
		{
			// 1. Get Link Group
			LinkLayer::PLINKGRP grp = GetLinkGroup(objectUID);
			if (!grp) return;	// no such link

			size_t groupLinkCount = grp->GetLinkCount();

			::NM::ODB::OBJECTUID nodeAGuiGroup = ::NM::ODB::INVALID_OBJECT_UID;
			::NM::ODB::OBJECTUID nodeBGuiGroup = ::NM::ODB::INVALID_OBJECT_UID;
			// gets the associated groups for each of the two nodes in the link group
			// (all links in grp share common nodes of course)
			grp->GetLinkGUIGroups(nodeAGuiGroup, nodeBGuiGroup);



			// remove GUIGroup -> Link lookup map entries
			// group A
			{
				GUIGRPLNKGMAP::iterator guiit = _guiGroupLinkGrpMap.find(nodeAGuiGroup);
				if (guiit != _guiGroupLinkGrpMap.end())
				{
					::std::vector<::std::shared_ptr<LinkGroup>>::iterator vlgit = ::std::find(guiit->second.begin(), guiit->second.end(), grp);
					if (vlgit != guiit->second.end())
					{
						guiit->second.erase(vlgit);
					}
#ifdef DEBUG
					else
					{
						assert(false);
					}
#endif 
				}
			}
			// group B
			{
				GUIGRPLNKGMAP::iterator guiit = _guiGroupLinkGrpMap.find(nodeBGuiGroup);
				if (guiit != _guiGroupLinkGrpMap.end())
				{
					::std::vector<::std::shared_ptr<LinkGroup>>::iterator vlgit = ::std::find(guiit->second.begin(), guiit->second.end(), grp);
					if (vlgit != guiit->second.end())
					{
						guiit->second.erase(vlgit);
					}
#ifdef DEBUG
					else
					{
						assert(false);
					}
#endif 
				}
			}






			// if only one link in this link group, then delete link and link group
			if (groupLinkCount == 1)
			{
				::NM::ODB::OBJECTUID nodeA = ::NM::ODB::INVALID_OBJECT_UID;
				::NM::ODB::OBJECTUID nodeB = ::NM::ODB::INVALID_OBJECT_UID;

				grp->GetEndPointObjectUIDs(nodeA, nodeB);
				
				NODEGRPMAP::iterator nodeit = _nodeGrpMap.find(::std::make_pair(nodeA, nodeB));
				if (nodeit != _nodeGrpMap.end())
				{
					_nodeGrpMap.erase(nodeit);
				}

				LINKGROUPS::iterator groupit = _linkGroups.find(grp);
				LINKGRPMAP::iterator it = _linkGrpMap.find(objectUID);

				if (it != _linkGrpMap.end())
				{
					_linkGrpMap.erase(it);
				}

				//grp.reset();
				if (groupit != _linkGroups.end())
				{
					_linkGroups.erase(groupit);
				}


			}
			// just delete the link from the group as more links exist in the 
			// group and we dont want to delete them
			else
			{
				grp->DeleteLink(objectUID);
			}

					


			return;
		}
		/**
		*
		*
		*
		*/
		void LinkLayer::EditLinkControlPoints(::NM::ODB::OBJECTUID objectUID)
		{
			PLINK pLink = GetLink(objectUID);
			if (!pLink) return;

			// create state
			_linkLayerState.reset(new LinkLayerEditControlPointsState(this));

			std::static_pointer_cast<LinkLayerEditControlPointsState>(_linkLayerState)->StartEditLinkControlPoints(pLink);

			// redraw without this link
			::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget;
			GetRenderTarget(pRenderTarget);
			OnRender(pRenderTarget, true);
			Present();
			
			return;
		}
		/**
		* BuildLinkCache
		*
		* loop through all edges in db
		* call insert link on linkgroup instance to add the link and create LinkGroup if it does not exist
		*/
		void LinkLayer::BuildLinkCache()
		{
			::std::wstring table = L"edgetable";
			OBJECTUID objectUID = _odb->GetFirstObject(table);
			while (objectUID != INVALID_OBJECT_UID)
			{
				// get connected vertex A
				::NM::ODB::OBJECTUID intfA = _odb->GetValue(objectUID, L"interfaceUID_A")->Get<::NM::ODB::OBJECTUID>();
				assert(intfA != ::NM::ODB::INVALID_OBJECT_UID);
				::NM::ODB::OBJECTUID nodeA = _odb->GetValue(intfA, L"vertexUID")->Get<::NM::ODB::OBJECTUID>();
				assert(nodeA != ::NM::ODB::INVALID_OBJECT_UID);
				// get connected vertex b
				::NM::ODB::OBJECTUID intfB = _odb->GetValue(objectUID, L"interfaceUID_B")->Get<::NM::ODB::OBJECTUID>();
				assert(intfB != ::NM::ODB::INVALID_OBJECT_UID);
				::NM::ODB::OBJECTUID nodeB = _odb->GetValue(intfB, L"vertexUID")->Get<::NM::ODB::OBJECTUID>();
				assert(nodeB != ::NM::ODB::INVALID_OBJECT_UID);

				//OBJECTUID nodeA = _odb->GetValue(objectUID, L"vertex_a_uid")->Get<::NM::ODB::ODBUID>();
				//OBJECTUID nodeB = _odb->GetValue(objectUID, L"vertex_b_uid")->Get<::NM::ODB::ODBUID>();

				PLINK pLink = InsertLink(objectUID, nodeA, nodeB);
				// next link
				objectUID = _odb->GetNextObject(objectUID);
			}
			return;
		}
		/**
		* OnRender
		*
		* Create/Recreate bitmap if is requires it, either way - make a usable bitmap available.
		*/
		HRESULT LinkLayer::OnRender(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> &pRenderTarget, bool bRedraw)
		{
			HRESULT hr = S_OK;			
			// unless forced redraw, we dont need to as its static content
			if (!bRedraw) return S_OK;
			//OutputDebugString(L"\nLinkLayer::OnRender");

			assert(pRenderTarget);

			// ensure shared resources already created
			if (
				(! _linkDrawResources->_nodebrushRed)				||
				(! _linkDrawResources->_nodebrushControlPtsLine)	||
				(! _linkDrawResources->_nodebrushSelected)			||
				(! _linkDrawResources->_nodebrushStateDown)			||
				(! _linkDrawResources->_nodebrushStateUnknown)	
				)				
			{
				_linkDrawResources->RefreshContent(pRenderTarget);
			}

			pRenderTarget->BeginDraw();
			pRenderTarget->Clear();
			
#ifdef DEBUG
#pragma region DrawLayerText
						
			pTextWhiteBrush_.Reset();
				hr = pRenderTarget->CreateSolidColorBrush(
					D2D1::ColorF(D2D1::ColorF::White, 0.1f),
					&pTextWhiteBrush_
					);
			
			D2D1_SIZE_F size = pRenderTarget->GetSize();
			float dpiScaleX_ = 0;;
			float dpiScaleY_ = 0;
			pRenderTarget->GetDpi(&dpiScaleX_, &dpiScaleY_);
			D2D1_RECT_F layoutRect = D2D1::RectF(
				static_cast<FLOAT>(8),// / dpiScaleX_,
				static_cast<FLOAT>(-20),// / dpiScaleY_,
				static_cast<FLOAT>(size.width - 8),// / dpiScaleX_,
				static_cast<FLOAT>(size.height)// / dpiScaleY_
				);
			
			// transform
			::D2D1::Matrix3x2F currentTransform;
			pRenderTarget->GetTransform(&currentTransform);
			pRenderTarget->SetTransform(::D2D1::Matrix3x2F::Identity());

			pRenderTarget->DrawText(
				wszText_,        // The string to render.
				cTextLength_,    // The string's length.
				pTextFormat_.Get(),    // The text format.
				layoutRect,       // The region of the window where the text will be rendered.
				pTextWhiteBrush_.Get()     // The brush used to draw the text.
				);

			pRenderTarget->SetTransform(&currentTransform);

#pragma endregion DrawLayerText
			
#endif

			if (_linkLayerState)
			{
				_linkLayerState->OnRender(pRenderTarget, _linkDrawResources);
			}
			
			LINKGROUPS::iterator groupit = _linkGroups.begin();
			while (groupit != _linkGroups.end())
			{
				(*groupit)->RenderLinkGroup(pRenderTarget, _currentLayerUID, _linkDrawResources);
				++groupit;
			}

			hr = pRenderTarget->EndDraw();

			return hr;
		}
		/**
		* MultiSelectChange
		*
		* Called from SelectedObjectsSvc
		*/
		void LinkLayer::MultiSelectChange(bool bMultiSelect)
		{
			return;
		}
		/**
		* SelectedObjectsUpdate
		*
		* Called from SelectedObjectsSvc
		*/
		void LinkLayer::SelectedObjectsUpdate()
		{
			if (!_bSupressSelectedObjectUpdates)
			{
				::std::vector<::NM::ODB::OBJECTUID> selectedObjects;
				_selectedObjectsSvc->GetSelected(selectedObjects);

				for each (::NM::ODB::OBJECTUID objectUID in _selectedObjects)
				{
					PLINK pLink = GetLink(objectUID);
					if (pLink)
						pLink->SetSelectedState(false);
				}
				_selectedObjects.clear();

				// copy new selected
				for each (::NM::ODB::OBJECTUID objectUID in selectedObjects)
				{
					PLINK pLink = GetLink(objectUID);
					if (pLink)
					{
						pLink->SetSelectedState(true);
						_selectedObjects.push_back(objectUID);
					}
				}
			}
			::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget;
			GetRenderTarget(pRenderTarget);
			OnRender(pRenderTarget, true);
			Present();

			return;
		}
		/**
		* DatabaseUpdate
		*
		* Called from Database cache when update recieved for db updates we are interested in.
		*/
		void LinkLayer::DatabaseUpdate()
		{
			size_t qSize = _updateQ->QueueSize();
			while (!_updateQ->Empty())
			{
				::std::shared_ptr<::NM::ODB::DatabaseUpdateRecord> record = _updateQ->GetNextRecord();
				::std::wstring attrname = record->GetObjectAttributeName();
				::NM::ODB::Value val = record->GetNewValue();

				// create link
				if (attrname == L"create")
				{
					assert(false);
				}
				// delete link
				else if (attrname == L"delete")
				{
					// call deletelink/group
					DeleteLink(record->GetObjectUID());
				}
				// link admin/operational state
				else if (attrname == L"inservice")
				{
					// get link group for link ObjectUID
					LINKGRPMAP::iterator it = _linkGrpMap.find(record->GetObjectUID());
					if (it != _linkGrpMap.end())
					{
						// call link group set operational state
						it->second->SetLinkOperationalState(record->GetObjectUID(),( val.Get<::NM::ODB::ODBBool>() ? Link::LinkOperationalState::Up : Link::LinkOperationalState::Down));
					}
				}	
				// link color
				else if (attrname == L"color")
				{
					PLINK pLink = GetLink(record->GetObjectUID());
					::NM::ODB::real_color linkColor = val.Get<::NM::ODB::ODBColor>();

					pLink->SetLineColor(
						::D2D1::ColorF(
							(UINT32)(
								linkColor.GetBlue() |
								linkColor.GetGreen() << 8 |
								linkColor.GetRed() << 16
								), 1.0f
							));
		

					//pLink->SetLineColor(::D2D1::ColorF((UINT32)val.Get<::NM::ODB::ODBColor>(), 1.0f));
					::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget;
					GetRenderTarget(pRenderTarget);
					pLink->RefreshContent(pRenderTarget);
				}
				// link lindewidth change
				else if (attrname == L"linewidth")
				{
					PLINK pLink = GetLink(record->GetObjectUID());
					pLink->SetLineWidth(static_cast<float>(val.Get<::NM::ODB::ODBInt>()));
				}
				// node x,y pos change
				else if ((attrname == L"xpos") || (attrname == L"ypos"))
				{
					if (_odb->GetObjectTypeName(record->GetObjectUID()) == L"group")
					{
						GUIGRPLNKGMAP::iterator bit = _guiGroupLinkGrpMap.find(record->GetObjectUID());
						if (bit != _guiGroupLinkGrpMap.end())
						{
							for each (::std::shared_ptr<LinkGroup> linkGroup in bit->second)
							{
								linkGroup->SetNodePositions({ 0, 0 }, { 0, 0 });
							}
						}
					}
					else
					{
						//this is a node pos change
						//::NM::ODB::IAdjacencyMatrix adjm = _odb->GetAdjacencyMatrix();
						::std::vector<::NM::ODB::OBJECTUID> vec;
						_adjm->GetVertexEdges(record->GetObjectUID(), vec);
						// for each link in vec, ammend endpoint for node
						for each(::NM::ODB::OBJECTUID link in vec)
						{
							LINKGRPMAP::iterator it = _linkGrpMap.find(link);
							if (it != _linkGrpMap.end())
							{
								// get the link instance 
								::std::shared_ptr<Link> pLink = it->second->GetLink(link);
								if (pLink)
								{
									it->second->SetNodePositions({ 0, 0 }, { 0, 0 });

									// get the link current endpoints
									//D2D1_POINT_2F posA = { 0, 0 };
									//D2D1_POINT_2F posB = { 0, 0 };
									//pLink->GetEndPoints(posA, posB);

									//// get node A/B for this link from the group
									//::NM::ODB::OBJECTUID nodeA;
									//::NM::ODB::OBJECTUID nodeB;
									//it->second->GetEndPointObjectUIDs(nodeA, nodeB);

									//// compare nodeIDs to find the one to be updated
									//if (nodeA == record->GetObjectUID())
									//{
									//	// update xpos or ypos on NodeA
									//	if (attrname == L"xpos")
									//		posA.x = static_cast<float>(val.Get<::NM::ODB::ODBInt>());
									//	else
									//		posA.y = static_cast<float>(val.Get<::NM::ODB::ODBInt>());
									//}
									//else if (nodeB == record->GetObjectUID())
									//{
									//	// update xpos or ypos on NodeB
									//	if (attrname == L"xpos")
									//		posB.x = static_cast<float>(val.Get<::NM::ODB::ODBInt>());
									//	else
									//		posB.y = static_cast<float>(val.Get<::NM::ODB::ODBInt>());
									//}
									// set the new endpoints
									//it->second->SetNodePositions(posA, posB);
								}
							}
						}
					}

				}
			}

			// render...
			::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget;
			GetRenderTarget(pRenderTarget);
			OnRender(pRenderTarget, true);
			Present();
			return;
		}
		/**
		* ProcessCommandMessage
		*
		* Any command messages from Menu buttons or popupmenus etc
		*
		* Return TRUE if message was processed
		*/
		BOOL LinkLayer::ProcessCommandMessage(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
		{
			BOOL bProcessed = FALSE;
			CCmdUI* pCmdUI = (CCmdUI*)pExtra;

			switch (nCode)
			{
			case CN_UPDATE_COMMAND_UI:
			{
				switch (nID)
				{
				//case ID_ENABLEOBJECT:
				//	pCmdUI->Enable(TRUE);
				//	bProcessed = TRUE;
				//	break;
				case 35000: // Disable/Enable toggle
				{
					PLINK pLink = GetLink(_selectedObjects[_selectedObjects.size() - 1]);
					if (pLink->IsInternalLink())
					{
						// assume group is collapsed as we have an internal link selected, so use the group operational state
						if (pLink->GetOperationalState() == Link::LinkOperationalState::Unknown)
						{
							pCmdUI->Enable(FALSE);
						}
						else
						{
							pCmdUI->Enable(TRUE);
						}
					}
					else
					{
						pCmdUI->Enable(TRUE);
					}

					bProcessed = TRUE;
				}
					break;

				case 35001: // Collapse Links Toggle
				{
					PLINK pLink = GetLink(_selectedObjects[_selectedObjects.size() - 1]);
					if (pLink)
					{
						PLINKGRP pGroup = GetLinkGroup(pLink->GetLinkUID());
						pCmdUI->Enable( (pGroup->GetLinkCount() > 1 ? TRUE : FALSE));
						bProcessed = TRUE;
					}
				}
					break;

				case 35002: // Distribute Links
				{	
					PLINK pLink = GetLink(_selectedObjects[_selectedObjects.size() - 1]);
					if (pLink->IsInternalLink())
					{
						pCmdUI->Enable(FALSE);
					}
					else
					{
						PLINKGRP grp = GetLinkGroup(pLink->GetLinkUID());
						if (grp->GetLinkGroupExpanded() == LinkGroup::LinkDrawState::Expanded)
						{
							pCmdUI->Enable(TRUE);
						}
						else
						{
							pCmdUI->Enable(FALSE);
						}
					}

					bProcessed = TRUE;
				}
					break;

				case 35003: // Reset Links
					pCmdUI->Enable(TRUE);
					bProcessed = TRUE;
					break;

				case 35004:   // delete link
				{
					PLINK pLink = GetLink(_selectedObjects[_selectedObjects.size() - 1]);
					pCmdUI->Enable(pLink->IsInternalLink() ? FALSE : TRUE);		
					bProcessed = TRUE;

				}
					break;

				}
			}
			break;

			case CN_COMMAND:
			{
				switch (nID)
				{
					//case ID_ENABLEOBJECT:
				case 35000:		// Disable/Enable toggle
				{
					PLINK pLink = GetLink(_selectedObjects[_selectedObjects.size() - 1]);
					// test if link is part of a group or link isinternallink, and if the group is collapsed
					// either take the group state or the link state, dont take from DB as this will be 
					// sync'd with DB.
					::NM::ODB::LockHandle lock = _updateCache->GetLock();
					if (pLink->IsInternalLink())
					{
						// assume group is collapsed, so use the group operational state
						Link::LinkOperationalState opState = pLink->GetOperationalState();
						// get all links in group
						PLINKGRP grp = GetLinkGroup(pLink->GetLinkUID());
						::std::vector<::NM::ODB::OBJECTUID> linkids = grp->GetGroupLinksUID();
						switch (opState)
						{
						case Link::LinkOperationalState::Up:
							for each(::NM::ODB::OBJECTUID objectUID in linkids)
							{
								_odb->SetValue(objectUID, L"inservice", real_bool(false));
							}
							break;

						case Link::LinkOperationalState::Down:
							for each(::NM::ODB::OBJECTUID objectUID in linkids)
							{
								_odb->SetValue(objectUID, L"inservice", real_bool(true));
							}
							break;

						case Link::LinkOperationalState::Unknown:
							break;

						default:
							break;
						}
					}
					else
					{
						// dealing with a single link
						PLINK pLink = GetLink(_selectedObjects[_selectedObjects.size() - 1]);
						Link::LinkOperationalState opState = pLink->GetOperationalState();
						::NM::ODB::OBJECTUID objectUID = pLink->GetLinkUID();
						real_bool bEnabled = _odb->GetValue(objectUID, L"inservice")->Get<::NM::ODB::ODBBool>();
						bEnabled = !bEnabled;
						_odb->SetValue(objectUID, L"inservice", bEnabled);
					}

					bProcessed = TRUE;

				}
				break;

				case 35001:		// Collapse Links Toggle
				{
					PLINK pLink = GetLink(_selectedObjects[_selectedObjects.size() - 1]);
					PLINKGRP pGroup = GetLinkGroup(pLink->GetLinkUID());
					LinkGroup::LinkDrawState drawstate = pGroup->GetLinkGroupExpanded();
					if (drawstate == LinkGroup::LinkDrawState::Expanded)
						pGroup->SetLinkGroupExpanded(LinkGroup::LinkDrawState::Collapsed);	// set collapsed
					else
						pGroup->SetLinkGroupExpanded(LinkGroup::LinkDrawState::Expanded);	// set expanded

					::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget;
					GetRenderTarget(pRenderTarget);
					OnRender(pRenderTarget, true);
					Present();
					bProcessed = TRUE;

				}

				break;

				case 35002:		// Distribute Links
				{
					PLINK pLink = GetLink(_selectedObjects[_selectedObjects.size() - 1]);
					if (!pLink->IsInternalLink())
					{
						PLINKGRP grp = GetLinkGroup(pLink->GetLinkUID());
						grp->DistributeLinks();
						::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget;
						GetRenderTarget(pRenderTarget);
						OnRender(pRenderTarget, true);
						Present();
						bProcessed = TRUE;
					}
				}
				break;

				case 35003:		// Reset Links
				{
					PLINK pLink = GetLink(_selectedObjects[_selectedObjects.size() - 1]);
					pLink->ResetControlPoints();

					D2D1_POINT_2F cpA;
					D2D1_POINT_2F cpB;
					pLink->GetControlPoints(cpA, cpB);

					_odb->SetValue(pLink->GetLinkUID(), L"controlpointAxpos", ::NM::ODB::real_int(-1));
					_odb->SetValue(pLink->GetLinkUID(), L"controlpointAypos", ::NM::ODB::real_int(-1));
					_odb->SetValue(pLink->GetLinkUID(), L"controlpointBxpos", ::NM::ODB::real_int(-1));
					_odb->SetValue(pLink->GetLinkUID(), L"controlpointBypos", ::NM::ODB::real_int(-1));

					Link::LINKPATHPOINTS points;
					pLink->GetLinkPathPoints(_spatialHash->GetSpatialGridSize(), points);
					_spatialHash->UpdateSpatialHashNode(pLink->GetLinkUID(), points);

					::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget;
					GetRenderTarget(pRenderTarget);
					OnRender(pRenderTarget, true);
					Present();
					bProcessed = TRUE;
				}
				break;

				case 35004:		// delete link
				{
					// dealing with a single link
					PLINK pLink = GetLink(_selectedObjects[_selectedObjects.size() - 1]);
					_odb->DeleteObject(pLink->GetLinkUID());
					bProcessed = TRUE;
				}
				break;


				}
			}
			break;

			default: 
				break;
			}

			return bProcessed;
		}
		/**
		*
		*
		* An application returns zero if it processes this message.
		*/
		LRESULT LinkLayer::ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam)
		{
			POINT p = { 0, 0 };
			p.x = GET_X_LPARAM(lParam);
			p.y = GET_Y_LPARAM(lParam);
			bool bRedraw = false;

			::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget;
			GetRenderTarget(pRenderTarget);

			LRESULT lResult = 1;			
			//OutputDebugString(L"\nLinkLayer::ProcessMessage");
			switch (message)
			{
			case NVLM_GROUPDISPLAYCHANGE:
				_currentLayerUID = *((GUID*)wParam);
			case WM_SIZE:
			{
				_linkDrawResources->RefreshContent(pRenderTarget);
				LINKGROUPS::iterator groupit = _linkGroups.begin();
				while (groupit != _linkGroups.end())
				{
					(*groupit)->RefreshContent(pRenderTarget);
					++groupit;
				}

				OnRender(pRenderTarget, true);
			}
				lResult = 1;	// other layers need this although the parent window ignores this on WM_SIZE
				break;


			case WM_LBUTTONDBLCLK:
				lResult = OnLeftButtonDblClick(p);
				break;

			case WM_LBUTTONUP:
				if (_linkLayerState)
				{
					lResult = _linkLayerState->ProcessMessage(message, wParam, lParam, this, pRenderTarget, bRedraw);
				}
				else
				{
					lResult = OnLeftButtonUp(p);
				}
				break;

			case WM_LBUTTONDOWN:
				if (_linkLayerState)
				{
					lResult = _linkLayerState->ProcessMessage(message, wParam, lParam, this, pRenderTarget, bRedraw);
				}
				else
				{
					lResult = OnLeftButtonDown(p);
				}

				break;

			case WM_MOUSEMOVE:
				if (_linkLayerState)
				{
					lResult = _linkLayerState->ProcessMessage(message, wParam, lParam, this, pRenderTarget, bRedraw);
				}
				break;

			case WM_MOUSEHOVER:
			{
				POINT p = { 0 };
				p.x = GET_X_LPARAM(lParam);
				p.y = GET_Y_LPARAM(lParam);
				PLINK pLink = GetLinkFromPoint(p);
				if (pLink)
				{
					::NM::ODB::OBJECTUID uid = pLink->GetLinkUID();
				
					if (!pLink->IsInternalLink())
					{
						SetToolTipTitle(_odb->GetValue(uid, L"shortname")->Get<::NM::ODB::ODBWString>());

						// get connected vertex A
						::NM::ODB::OBJECTUID intfA = _odb->GetValue(uid, L"interfaceUID_A")->Get<::NM::ODB::OBJECTUID>();
						assert(intfA != ::NM::ODB::INVALID_OBJECT_UID);
						::NM::ODB::OBJECTUID vertexA = _odb->GetValue(intfA, L"vertexUID")->Get<::NM::ODB::OBJECTUID>();
						assert(vertexA != ::NM::ODB::INVALID_OBJECT_UID);
						// get connected vertex b
						::NM::ODB::OBJECTUID intfB = _odb->GetValue(uid, L"interfaceUID_B")->Get<::NM::ODB::OBJECTUID>();
						assert(intfB != ::NM::ODB::INVALID_OBJECT_UID);
						::NM::ODB::OBJECTUID vertexB = _odb->GetValue(intfB, L"vertexUID")->Get<::NM::ODB::OBJECTUID>();
						assert(vertexB != ::NM::ODB::INVALID_OBJECT_UID);



						//::NM::ODB::OBJECTUID  vauid = _odb->GetValue(uid, L"vertex_a_uid")->Get<::NM::ODB::ODBUID>();
						//::NM::ODB::OBJECTUID  vbuid = _odb->GetValue(uid, L"vertex_b_uid")->Get<::NM::ODB::ODBUID>();


						::std::wstring connectedText = L"Connected Nodes";
						connectedText += L"\r\n" + _odb->GetValue(vertexA, L"shortname")->Get<::NM::ODB::ODBWString>();
						connectedText += L"\r\n" + _odb->GetValue(vertexB, L"shortname")->Get<::NM::ODB::ODBWString>();

						SetToolTipDescription(connectedText);
					}
					else
					{
						// deal with multiple links
					}
					return 0;
				}
				return 1;
			}
			break;

			case NVLM_LOSTFOCUS:
				if (_linkLayerState)
				{
					std::static_pointer_cast<LinkLayerEditControlPointsState>(_linkLayerState)->EndEditLinkControlPoints();
					_linkLayerState.reset();
					bRedraw = true;
				}
				break;

			case WM_RBUTTONDOWN:
				lResult = OnRightButtonDown(p);
				/*if (lResult == 0)
					bRedraw = true;*/
				break;

			case WM_RBUTTONUP:
				lResult = OnRightButtonUp(p);
				if (lResult == 0)
					bRedraw = true;
				break;

			case WM_COMMAND:
				OutputDebugString(L"\nWM_COMMAND..WM_COMMAND..");
				break;

			default:
				break;
			}

			if (bRedraw)
			{
				OnRender(pRenderTarget, true);
				Present();
			}

			return lResult;
		}
		/**
		*
		*
		*
		*/
		LinkLayer::PLINK LinkLayer::GetLinkFromPoint(POINT p)
		{
			//::std::vector<::NM::ODB::OBJECTUID> vec = _spatialHash->IsCaretOnObject(p);
			::std::vector<::NM::ODB::OBJECTUID> vec = _spatialHash->GetPotentialObjects(p);
			for each (::NM::ODB::OBJECTUID objectUID in vec)
			{
				PLINKGRP pLinkGrp = GetLinkGroup(objectUID);
				//assert(pLinkGrp); // should always have a link group for a link in spatialhash, 
				// with new spatial hash, the returned items will be anything in the cell so could be nodes,
				// which means we ignore as we dont have them stored in the local cache for links obviously
				if (pLinkGrp)
				{
					if (pLinkGrp->IsInGroup(_currentLayerUID))
					{
						PLINK pLink = GetLink(objectUID);
						if (pLink)
						{
							if (pLink->IsPointOnLine(::D2D1::Point2F(static_cast<float>(p.x), static_cast<float>(p.y))))
							{
								// do nothing yet
								return pLink;
							}
						}
					}
				}
			}

			return nullptr;
		}
		/**
		*
		*
		* Return 0 if we processed. actually we dont do a lot, just confirm if this click was on a link or not 
		*/
		LRESULT LinkLayer::OnRightButtonDown(POINT p)
		{			
			return (GetLinkFromPoint(p) ? 0 : -1);
		}
		/**
		*
		*
		* Return 0 if we processed.
		*/
		LRESULT LinkLayer::OnRightButtonUp(POINT p)
		{
			LRESULT lResult = 1;
			PLINK pLink = GetLinkFromPoint(p);
			if (!pLink)	return lResult;

			// toggle collapsed state  CTRL-Yes SHIFT-No
			///////////////////////////////////////////////////////////////////////////////////////
			if (IsControlKeyPressed() && !IsShiftKeyPressed())
			{
				PLINKGRP pGroup = GetLinkGroup(pLink->GetLinkUID());
				LinkGroup::LinkDrawState drawstate = pGroup->GetLinkGroupExpanded();
				if (drawstate == LinkGroup::LinkDrawState::Expanded)
					pGroup->SetLinkGroupExpanded(LinkGroup::LinkDrawState::Collapsed);	// set collapsed
				else
					pGroup->SetLinkGroupExpanded(LinkGroup::LinkDrawState::Expanded);	// set expanded

				lResult = 0;
			}
			// distribute links CTRL-Yes SHIFT-Yes
			///////////////////////////////////////////////////////////////////////////////////////
			else if (IsControlKeyPressed() && IsShiftKeyPressed())
			{
				PLINKGRP pGroup = GetLinkGroup(pLink->GetLinkUID());
				pGroup->DistributeLinks();
				lResult = 0;
			}
			// context menu - just a right click
			///////////////////////////////////////////////////////////////////////////////////////
			else
			{
				ClearSelectedObjects();
				DoSelectObject(pLink);
				lResult = OnContextMenu(p);
			}

			return lResult;
		}
		/**
		*
		*
		* Return 0 if processed
		*/
		LRESULT LinkLayer::OnContextMenu(POINT p)
		{
			LRESULT lResult = 1;
			PLINK pLink = GetLinkFromPoint(p);
			if (!pLink)	return lResult;

			::std::wstring menutext;
			HWND hWnd = GetHWND();
			CMenu menu;
			CMenu submenu;

			// create menus
			menu.CreateMenu();
			submenu.CreatePopupMenu();
			menu.AppendMenuW(MF_POPUP | MF_STRING | MF_ENABLED, (UINT_PTR)submenu.m_hMenu, L"LinkLayerMenu");

			// add menu itmes
			Link::LinkOperationalState opState = pLink->GetOperationalState();
			submenu.AppendMenuW(MF_STRING, 35000, (opState == Link::LinkOperationalState::Up ? L"Disable (Admin State)" : L"Enable (Admin State)"));

			PLINKGRP pGroup = GetLinkGroup(pLink->GetLinkUID());
			LinkGroup::LinkDrawState drawstate = pGroup->GetLinkGroupExpanded();
			submenu.AppendMenuW(MF_STRING, 35001, (drawstate == LinkGroup::LinkDrawState::Expanded ? L"Collapse Links" : L"Expand Links"));

			submenu.AppendMenuW(MF_STRING, 35002, L"Distribute Links");

			submenu.AppendMenuW(MF_STRING, 35003, L"Reset Link (Linear)");

			submenu.AppendMenuW(MF_STRING, 35004, L"Delete Link");


			// show popup menu
			POINT screenPoint;
			GetCursorPos(&screenPoint);

			CWnd* pWnd = CWnd::FromHandle(hWnd);
			CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;
			if (pPopupMenu->Create(pWnd, screenPoint.x, screenPoint.y, submenu.m_hMenu, TRUE, FALSE))
			{
				lResult = 1;
			}

			return lResult;
		}
		/**
		*
		*
		* Return 0 if we processed.
		*/
		LRESULT LinkLayer::OnLeftButtonDown(POINT p)
		{
			LRESULT lResult = 1;

			PLINK pLink = GetLinkFromPoint(p);
			if (pLink)
			{
				// do nothing yet
				return 0;
			}

			// click point not on any link, so if control not down then deselect all selected links
			if (IsControlKeyPressed()) return lResult;

			if (GetSelectedLinkCount() == 0) return lResult;

			ClearSelectedObjects();

			::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget;
			GetRenderTarget(pRenderTarget);
			OnRender(pRenderTarget, true);
			Present();

			// we return -1 as not processed as this click was not on any links, we process to 
			// remove selection but as this click maybe on a lower layer object we need to let it pass through
			// returning 0 would prevent the message continuing down the message pipeline
			return lResult;
		}
		/**
		* OnLeftButtonUp
		*
		* An application returns zero if it processes this message.
		*/
		LRESULT LinkLayer::OnLeftButtonUp(POINT p)
		{
			LRESULT lResult = 1;
			
			PLINK pLink = GetLinkFromPoint(p);
			if (pLink)
			{
				DoSelectObject(pLink);
				return 0;
			}

			return lResult;
		}
		/**
		* OnLeftButtonDblClick
		*
		* An application returns zero if it processes this message.
		*/
		LRESULT LinkLayer::OnLeftButtonDblClick(POINT p)
		{
			LRESULT lResult = 1;
			//::std::vector<::NM::ODB::OBJECTUID> vec = _spatialHash->IsCaretOnObject(p);
			//for each (::NM::ODB::OBJECTUID objectUID in vec)
			//{
			//	PLINK pLink = GetLink(objectUID);
			//	if (pLink)
			//	{
			//		if (pLink->IsPointOnLine(::D2D1::Point2F(static_cast<float>(p.x), static_cast<float>(p.y))))
			//		{
			::NM::ODB::OBJECTUID objectUID = ::NM::ODB::INVALID_OBJECT_UID;
			
			PLINK pLink = GetLinkFromPoint(p);
			if (pLink)
			{
				objectUID = pLink->GetLinkUID();
				if (IsControlKeyPressed() && IsShiftKeyPressed())
				{
					// reset biezer control points
					//PLINK pLink = GetLink(objectUID);
					//if (pLink)
					{
						pLink->ResetControlPoints();
						D2D1_POINT_2F cpA;
						D2D1_POINT_2F cpB;
						pLink->GetControlPoints(cpA, cpB);

						_odb->SetValue(objectUID, L"controlpointAxpos", ::NM::ODB::real_int(-1));
						_odb->SetValue(objectUID, L"controlpointAypos", ::NM::ODB::real_int(-1));
						_odb->SetValue(objectUID, L"controlpointBxpos", ::NM::ODB::real_int(-1));
						_odb->SetValue(objectUID, L"controlpointBypos", ::NM::ODB::real_int(-1));

						Link::LINKPATHPOINTS points;
						pLink->GetLinkPathPoints(_spatialHash->GetSpatialGridSize(), points);
						_spatialHash->UpdateSpatialHashNode(pLink->GetLinkUID(), points);
						//_spatialHash->UpdateSpatialHashNode(pLink);

					}

					::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget;
					GetRenderTarget(pRenderTarget);
					OnRender(pRenderTarget, true);
					Present();
				}
				else
				{
					// edit ObjectUID Control Points
					EditLinkControlPoints(objectUID);
				}
				// found it and processed - exit.
				return 0;

			}
			//		}
			//	}
			//}
			return lResult;

		}
		/**
		*
		*
		*
		*/
		void LinkLayer::DoSelectObject(PLINK pLink)
		{
			/*
			
			Need to see if not ctrl, if so, then get new link uid and apply direct to selected services without deslecting first
			the selected service will auto clear all selected and just selected the new one so no double updates, one for clear(no selected), 
			then one for selected objects which causing flickering on properties as it shows default screen on no select then flickers to show the
			new select, one update will cause the properties window to go from old select to new select in one draw.

			*/
			::NM::ODB::OBJECTUID objectUID = pLink->GetLinkUID();

			// check for same link click without ctrl
			if ((_selectedObjects.size() == 1) && !IsControlKeyPressed())
			{
				if (_selectedObjects[0] == objectUID)
					return;
			}
			
			// if ctrl not pressed, deselect all internal objects
			if (!IsControlKeyPressed())
			{
				for each (::NM::ODB::OBJECTUID objectUID in _selectedObjects)
				{
					PLINK pLink = GetLink(objectUID);
					if (pLink)
						pLink->SetSelectedState(false);
				}
				_selectedObjects.clear();
			}
		
			_bSupressSelectedObjectUpdates = true;
			::std::vector<::NM::ODB::OBJECTUID>::iterator selectit = ::std::find(_selectedObjects.begin(), _selectedObjects.end(), objectUID);
			// found due to already selected & control key down, so deselect only this object preserving other selections
			if (selectit != _selectedObjects.end())
			{
				// set object selected state to false (not selected)
				PLINK pLink = GetLink(objectUID);
				if (pLink)
					pLink->SetSelectedState(false);
				// delselect internally
				_selectedObjects.erase(selectit);
				// deselect globally
				_selectedObjectsSvc->Deselect(objectUID);
			}
			// not currently selected, so select the object, check ctrl key state
			else
			{
				// add object to internal selected
				_selectedObjects.push_back(objectUID);
				// get a list of the globally selected
				::std::vector<::NM::ODB::OBJECTUID> globalSelected;
				// only get global selected is ctrl key is pressed, if not we overwrite existing selections
				if (IsControlKeyPressed())
				{
					_selectedObjectsSvc->GetSelected(globalSelected);
				}
				// set internal link object selected state to true (selected)
				PLINK pLink = GetLink(objectUID);
				if (pLink)
					pLink->SetSelectedState(true);
				// add the new internal selected link to the global list
				globalSelected.push_back(objectUID);
				// and send new global list back to the selected service
				_selectedObjectsSvc->Select(globalSelected);			
			}

			_bSupressSelectedObjectUpdates = false;
			//// redraw
			//::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget;
			//GetRenderTarget(pRenderTarget);
			//OnRender(pRenderTarget, true);
			//Present();
			return;
		}
		/**
		*
		*
		*
		*/
		void LinkLayer::ClearSelectedObjects()
		{
			for each (::NM::ODB::OBJECTUID objectUID in _selectedObjects)
			{
				PLINK pLink = GetLink(objectUID);
				if (pLink)
					pLink->SetSelectedState(false);
			}

			//_selectedObjectsSvc->Deselect(_selectedObjects);
			_selectedObjectsSvc->Clear();
			_selectedObjects.clear();
			return;
		}
		/**
		*
		*
		*
		*/
		bool LinkLayer::IsControlKeyPressed()
		{
			return (GetKeyState(VK_CONTROL) < 0);

			/*return ((GetAsyncKeyState(VK_CONTROL) & 0x8000) == 1);*/
		}
		/**
		*
		*
		*
		*/
		bool LinkLayer::IsShiftKeyPressed()
		{
			return (GetKeyState(VK_SHIFT) < 0);

			/*		return ((GetAsyncKeyState(VK_SHIFT) & 0x8000) == 1);*/
		}
		/**
		*
		*
		*
		*/
		bool LinkLayer::IsLeftButtonPressed()
		{
			return (GetKeyState(VK_LBUTTON) < 0);
		}




		// ns
	}
}

