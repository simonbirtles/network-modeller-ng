#include "stdafx.h"
#include "BitmapCache.h"
#include "Node.h"
#include "NodeLayer.h"
#include "NetworkViewSpatialHash.h"

#include "NodeLayerState.h"
#include "NodeIdleState.h"
#include "NodeDraggingState.h"
#include "NodeSelectState.h"

#include "IServiceRegistry.h"							// registry interface
#include "ISelectedObjects.h"							// holds references to currently selected objects application wide
#include "Interfaces\IObjectDatabase.h"					// for client interface to core database 
#include "Interfaces\IAttribute.h"						// interface to an attribute copy
#include "Interfaces\IDatabaseUpdate.h"					// for client interface for core database update notifications
#include "IAdjacencyMatrix.h"

#include <algorithm>

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
		NodeLayer::NodeLayer() :
			_bmpCache(nullptr),
			_spatialGridSize(64),  // default, actual value is assigned in init
			_leftButtonDown(false),
			_dragging(false),
			_lastMousePoint({ 0 }),
			_currentState(nullptr),
			_lastMouseDownPoint({ -1,-1 }),
			_currentLayerUID(::NM::ODB::INVALID_OBJECT_UID),
			_bSupressButtonUpProcessing(false),
			_bSupressSelectedObjectUpdates(false)
		{
		}
		/**
		*
		*
		*
		*/
		NodeLayer::~NodeLayer()
		{
			_selectedObjectsSvc->DeRegisterClientObserver(this);
			_updateCache->DeleteClientUpdatesQueue(this);
			DeleteLocalNodeCache();
		}
		/**
		* DeleteLocalNodeCache
		*
		* Removes all nodes in this layers cache 
		* Deletes the node instances and clears out the caches/lookup tables
		*/
		void NodeLayer::DeleteLocalNodeCache()
		{
			UID_NODE_MAP::iterator uidit = _nodeUIDLookup.begin();
			while (uidit != _nodeUIDLookup.end())
			{
				uidit->second.reset();
				//uidit->second = nullptr;
				++uidit;
			}
			_nodeUIDLookup.clear();

			::std::map<LayerState, NodeLayerState*>::iterator stateit = _states.begin();
			while (stateit != _states.end())
			{
				delete stateit->second;
				stateit->second = nullptr;
				++stateit;
			}
			_states.clear();

			return;
		}
		/**
		* Initialise
		*
		* Called from baselayer which is called upon window creation of containing window
		*/
		bool NodeLayer::Initialise()
		{
			OutputDebugString(L"\nNodeLayer::Initialise");
			HRESULT hr = S_OK;

			_spatialHash = GetSpatialHashPtr();
			if(!_spatialHash)
				throw ::std::runtime_error("Spatial Hash Service Not Available");

			

			// if the global registry is not available we are in trouble
			if (!reg)
				throw ::std::runtime_error("Application Registry Not Available, Cannot Continue.");

			// Get The Database Handle and save
			_odb.reset(reinterpret_cast<NM::ODB::IObjectDatabase*>(reg->GetClientInterface(L"ODB")));
			if (!_odb)
				throw ::std::runtime_error("Database Not Available, Cannot Continue.");

			// Get the handle to the database update cache
			_updateCache.reset( reinterpret_cast<::NM::ODB::IDatabaseUpdate*>(reg->GetClientInterface(L"ObjectUpdateCache")) );
			if (!_updateCache)
				throw ::std::runtime_error("Database UpdateCache Not Available, Cannot Continue.");

			_selectedObjectsSvc.reset((reinterpret_cast<NM::ISelectedObjects*>(reg->GetClientInterface(L"SELECTEDOBJECTS"))));
			if (!_selectedObjectsSvc)
				throw ::std::runtime_error("Selected Objects Service Not Available, Cannot Continue.");
			
			_selectedObjectsSvc->RegisterClientObserver(this);

			_adjm.reset(static_cast<::NM::DataServices::IAdjacencyMatrix*>(reg->GetClientInterface(L"ADJMATRIX")));
			if (!_adjm)
				throw ::std::runtime_error("Adj. Matrix Service Not Available, Cannot Continue.");

			// create direct write factory, will pass this to nodes to draw text
			hr = DWriteCreateFactory(
				DWRITE_FACTORY_TYPE_SHARED,
				__uuidof(IDWriteFactory),
				reinterpret_cast<IUnknown**>(&pDWriteFactory_)
				);

#pragma region LayerText
			wszText_ = L"Node Layer";
			cTextLength_ = (UINT32)wcslen(wszText_);

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
				hr = pTextFormat_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
			}

			if (SUCCEEDED(hr))
			{
				hr = pTextFormat_->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
			}

#pragma endregion LayerText

			_spatialGridSize = _spatialHash->GetSpatialGridSize();
			_bmpCache = GetBitmapCachePtr();

			// get the current render target, we dont save it 
			// as it can change at anytime and become invalid
			//ID2D1BitmapRenderTarget* pTarget;
			_pRenderTarget.Reset();
			GetRenderTarget(_pRenderTarget);
			assert(_pRenderTarget);
			// builds local cache of node objects and initialises them to create bitmaps
			RefreshNodes(_pRenderTarget);
			
			// Init states
			_states[LayerState::Idle] = new NodeIdleState;
			_states[LayerState::Select] = new NodeSelectState;
			_states[LayerState::Drag] = new NodeDraggingState;			
			SetState(LayerState::Idle);


			// register for specific database updates
			::std::vector<::std::wstring> attr_list;
			attr_list.push_back(L"xpos");
			attr_list.push_back(L"ypos");
			attr_list.push_back(L"zpos");
			attr_list.push_back(L"imagewidth");
			attr_list.push_back(L"imageheight");
			attr_list.push_back(L"shortname");
		//	attr_list.push_back(L"imagefilename");
		//	attr_list.push_back(L"defaultrgb");
			attr_list.push_back(L"inservice");
			//attr_list.push_back(L"latency");
			attr_list.push_back(L"create");
			attr_list.push_back(L"delete");
			_updateQ = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Vertex, attr_list);
			_updateQ = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Group, attr_list);
			
			OnRender(_pRenderTarget, true);
			
			return true;
		}
		/**
		* RefreshNodes
		*
		* Create new node instances for each node in the global db, update local caches
		* Call init on the node instances to create bitmaps 
		* Update the spatial index with the position of the nodes
		* 
		*/
		void NodeLayer::RefreshNodes(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget)
		{
			DeleteLocalNodeCache();
			::std::multimap<size_t, ::NM::ODB::OBJECTUID> _nodeZOrder;	// multimap in case duplicate zpos until we change from int to somethng else dbl linked list?
			_bmpCache->RefreshContent(pRenderTarget);

			// create gui nodes

			// nodes
			::std::wstring table = L"vertextable";
			OBJECTUID objectUID = _odb->GetFirstObject(table);

			while (objectUID != INVALID_OBJECT_UID)
			{
				// create node class for this node				
				PNODE node = CreateNewNode(objectUID);
				// update local indexes
				real_int zpos = _odb->GetValue(objectUID, L"zpos")->Get<::NM::ODB::ODBInt>();
				_nodeZOrder.insert( ::std::pair<size_t, ::NM::ODB::OBJECTUID>(zpos, objectUID));
				_nodeUIDLookup[objectUID] = node;
				// create entry for spatial hash
				InsertSpatialHashNode(objectUID);
				
				// next object
				objectUID = _odb->GetNextObject(objectUID);
			} 
			
			table = L"grouptable";
			objectUID = _odb->GetFirstObject(table);

			while (objectUID != INVALID_OBJECT_UID)
			{				
				// create node class for this node				
				PNODE node = CreateNewNode(objectUID);
				// update local indexes
				real_int zpos = _odb->GetValue(objectUID, L"zpos")->Get<::NM::ODB::ODBInt>();
				_nodeZOrder.insert(::std::pair<size_t, ::NM::ODB::OBJECTUID>(zpos, objectUID));
				_nodeUIDLookup[objectUID] = node;
				// create entry for spatial hash
				InsertSpatialHashNode(objectUID);

				// next object
				objectUID = _odb->GetNextObject(objectUID);
			}
			
			// create z-order
			_nodeRenderOrder.clear(); // just in case
			for (::std::multimap<size_t, ::NM::ODB::OBJECTUID>::iterator it = _nodeZOrder.begin(); it != _nodeZOrder.end(); ++it)
				_nodeRenderOrder.push_back(it->second);
					

			return;
		}
		/**
		* CopyNode
		*
		* This function copied the NodeLayer internal node class instance, node class instance is just the gui representation
		* NOT the DB node instance
		*/
		NodeLayer::PNODE NodeLayer::CopyNode(::NM::ODB::OBJECTUID objectUID)
		{
			PNODE existingNode = GetNode(objectUID);
			if (!existingNode) return nullptr;

			PNODE copyNode = ::std::make_shared<Node>(*existingNode);
			copyNode->RefreshContent(_pRenderTarget);

			return copyNode;
		}
		/**
		* CreateNewNode
		*
		*
		*/
		NodeLayer::PNODE NodeLayer::CreateNewNode(::NM::ODB::OBJECTUID objectUID)
		{
			real_int xpos = _odb->GetValue(objectUID, L"xpos")->Get<::NM::ODB::ODBInt>();
			real_int ypos = _odb->GetValue(objectUID, L"ypos")->Get<::NM::ODB::ODBInt>();
			real_int zpos = _odb->GetValue(objectUID, L"zpos")->Get<::NM::ODB::ODBInt>();
			real_int imageWidth = _odb->GetValue(objectUID, L"imagewidth")->Get<::NM::ODB::ODBInt>();
			real_int imageHeight = _odb->GetValue(objectUID, L"imageheight")->Get<::NM::ODB::ODBInt>();
			real_string shortname = _odb->GetValue(objectUID, L"shortname")->Get<::NM::ODB::ODBWString>();
			real_uid groupUID = _odb->GetValue(objectUID, L"group")->Get<::NM::ODB::ODBUID>();
			real_bool inservice = true;

			::std::wstring imageFile;

			if (_odb->GetObjectTypeName(objectUID) == L"vertex")
			{
				//inservice = _odb->GetValue(objectUID, L"inservice")->Get<::NM::ODB::ODBBool>();
				//// get type
				//::NM::ODB::VertexType vType = static_cast<::NM::ODB::VertexType>(_odb->GetValue(objectUID, L"type")->Get<::NM::ODB::ODBInt>());
				//// set node image on L3/L2 device
				//switch (vType)
				//{
				//case ::NM::ODB::VertexType::Router:
				//	// l3
					imageFile = L"nodeblue.png";
				//	break;

				//case ::NM::ODB::VertexType::Switch:
				//	// l2
				//	imageFile = L"nodelightblue.png";
				//	break;

				//default:
				//	imageFile = L"unknownobject.png";
				//	break;
				//}				
			}
			else if (_odb->GetObjectTypeName(objectUID) == L"group")
			{
				real_bool inservice = true;
				imageFile = L"city.png";
			}
			else
			{
				real_bool inservice = false;
				imageFile = L"unknownobject.png";
			}
			
			// create gui node
			PNODE node = ::std::make_shared<Node>(objectUID, groupUID, xpos, ypos, imageWidth, imageHeight, shortname, imageFile, pDWriteFactory_, _bmpCache);
			node->RefreshContent(_pRenderTarget);
			node->SetAdminState((inservice ? NodeAdminState::Enabled : NodeAdminState::Disabled));

			return node;
		}
		/**
		*
		*
		*
		*/
		void NodeLayer::InsertSpatialHashNode(::NM::ODB::OBJECTUID objectUID)
		{
			PNODE pNode = GetNode(objectUID);
			Node::NODEPOINTS points;
			pNode->GetNodePoints(_spatialHash->GetSpatialGridSize(), points);
			_spatialHash->InsertSpatialHashNode(objectUID, points);
			return;		
		}
		/**
		*
		*
		*
		*/
		void NodeLayer::UpdateSpatialHashNode(::NM::ODB::OBJECTUID objectUID)
		{
			PNODE pNode = GetNode(objectUID);
			Node::NODEPOINTS points;
			pNode->GetNodePoints(_spatialHash->GetSpatialGridSize(), points);
			_spatialHash->UpdateSpatialHashNode(objectUID, points);
			return;

		}
		/**
		* DeleteSpatialHashNode
		*
		*
		*/
		void NodeLayer::DeleteSpatialHashNode(::NM::ODB::OBJECTUID objectUID)
		{
			_spatialHash->DeleteSpatialHashNode(objectUID);
			return;			
		}
		/** 
		* RefreshNodeContent
		*
		* Typically called when the window has changed, ie WM_SIZE
		* Any rendertarget change will cause this to be called - if its called in right places, its not automatic
		* Informs all node instances that they need to rebuild bitmaps 
		*/
		void NodeLayer::RefreshNodeContent(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget)
		{
			OutputDebugString(L"\nRefreshNodeContent");
			_bmpCache->RefreshContent(pRenderTarget);
			::std::vector<::NM::ODB::OBJECTUID>::iterator zit = _nodeRenderOrder.begin();
			while (zit != _nodeRenderOrder.end())
			{
				PNODE node = GetNode(*zit);
				if(node)
					node->RefreshContent(pRenderTarget);

				++zit;
			}
			return;
		}
		/**
		*
		*
		*
		*/
		NodeLayer::PNODE NodeLayer::GetNode(::NM::ODB::OBJECTUID objectUID)
		{
			UID_NODE_MAP::iterator it = _nodeUIDLookup.find(objectUID);
			if (it != _nodeUIDLookup.end())
				return it->second;

			return nullptr;
		}
		/**
		* OnRender
		*
		* Draw. 
		* Calls each node instance to draw its bitmap (image and text) on the 
		* RenderTarget. We only redraw if forced to or we think we need to
		*/
		HRESULT NodeLayer::OnRender(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> &pRenderTarget, bool bRedraw)
		{
			HRESULT hr = S_OK;
			//OutputDebugString(L"\nNodeLayer::OnRender");

			assert(pRenderTarget);
			// save current target;
			_pRenderTarget = pRenderTarget;
						
			if (bRedraw)
			{
				//OutputDebugString(L"\nNodeLayer::OnRender\t Redrawing Static Content");
				_pStaticBitmap.Reset();
				hr = pRenderTarget->CreateCompatibleRenderTarget(_pStaticBitmap.GetAddressOf());
				// transform
				::D2D1::Matrix3x2F sourceTargetTransform;
				pRenderTarget->GetTransform(&sourceTargetTransform);
				_pStaticBitmap->SetTransform(&sourceTargetTransform);

				_pStaticBitmap->BeginDraw();
				RenderNodes(_pStaticBitmap.Get());
				hr = _pStaticBitmap->EndDraw();

				_pBitmap.Reset();
				_pStaticBitmap->GetBitmap(_pBitmap.GetAddressOf());
			}

			/*******************************
			* BeginDraw
			*******************************/

			//OutputDebugString(L"\nNodeLayer::OnRender\t Copy Static Content & Drawing States");
			pRenderTarget->BeginDraw();
			pRenderTarget->Clear();

			//transform
			::D2D1::Matrix3x2F sourceTargetTransform;
			pRenderTarget->GetTransform(&sourceTargetTransform);
			pRenderTarget->SetTransform(::D2D1::Matrix3x2F::Identity());

			pRenderTarget->DrawBitmap(_pBitmap.Get());

			// transform
			pRenderTarget->SetTransform(&sourceTargetTransform);


			// allow current state to render if need be, i.e. dragging nodes.
			if (_currentState)
				_currentState->OnRender(pRenderTarget);

			hr = pRenderTarget->EndDraw();

			/*****************************
			* EndDraw
			*****************************/
			
			return hr;
		}
		/**
		* RenderNodes
		*
		* Calls each node to render itself
		*/
		void NodeLayer::RenderNodes(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget)
		{

			HRESULT hr = S_OK;

#ifdef DEBUG
#pragma region oldcode
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// draw layer identifier text

		

			hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 0.1f), &pTextWhiteBrush_);

			D2D1_SIZE_F size = pRenderTarget->GetSize();
			float dpiScaleX_ = 0;;
			float dpiScaleY_ = 0;
			pRenderTarget->GetDpi(&dpiScaleX_, &dpiScaleY_);
			D2D1_RECT_F layoutRect = D2D1::RectF(
				static_cast<FLOAT>(0),// / dpiScaleX_,
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

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma endregion oldcode

#endif

			::std::vector<::NM::ODB::OBJECTUID>::iterator zit = _nodeRenderOrder.begin();
			::NM::ODB::OBJECTUID currentGroup = GetCurrentLayerUID();
			while (zit != _nodeRenderOrder.end())
			{
				PNODE node = GetNode(*zit);
				if (node)
					node->Draw(pRenderTarget, currentGroup);

				++zit;
			}

			return;
		}
		/**
		* IsCaretOnObject
		*
		* Takes a client window x,y co-ord and uses spatial hash class to identify the potential nodes
		* in the virtual grid the point is in, then for each potential, the node class instance is
		* then asked to validate if the point is in its bitmap. A final list of potential nodes 
		* (ones which all match the point.. overlapping incl.) are then evaluted based on zpos
		* and the topmost objectuid is returned.
		*/
		::NM::ODB::OBJECTUID NodeLayer::IsCaretOnObject(POINT &clientWindowPoint)
		{
			::std::vector<::NM::ODB::OBJECTUID> potentialObjects = _spatialHash->GetPotentialObjects(clientWindowPoint);

			int lastZPos = -1;
			::NM::ODB::OBJECTUID selectedNode = ::NM::ODB::INVALID_OBJECT_UID;
			for each(::NM::ODB::OBJECTUID objectid in potentialObjects)
			{
				UID_NODE_MAP::iterator uidit = _nodeUIDLookup.find(objectid);
				if (uidit != _nodeUIDLookup.end())
				{
					if ((uidit->second->IsPointInBitmap(clientWindowPoint)) && uidit->second->GetGroup() == _currentLayerUID)
					{
						//OutputDebugString(L"\n");
						//OutputDebugString(_odb->GetValue(objectid, L"shortname")->Get<::NM::ODB::ODBWString>().c_str());
						int zpos = _odb->GetValue(objectid, L"zpos")->Get<::NM::ODB::ODBInt>();
						if (zpos > lastZPos)
						{
							lastZPos = zpos;
							selectedNode = objectid;
						}
					}
				}
			}
			//#ifdef DEBUG
			//			if (selectedNode != ::NM::ODB::INVALID_OBJECT_UID)
			//			{
			//				OutputDebugString(L"\nSelected Object :");
			//				OutputDebugString(_odb->GetValue(selectedNode, L"shortname")->Get<::NM::ODB::ODBWString>().c_str());
			//			}
			//			else
			//			{
			//				OutputDebugString(L"\nNo object found at click point");
			//			}
			//#endif // DEBUG
			return selectedNode;
		}
		/**
		* MultiSelectChange
		*
		* Called from SelectedObjectsSvc
		*/
		void NodeLayer::MultiSelectChange(bool bMultiSelect)
		{
			return;
		}
		/**
		* SelectedObjectsUpdate
		*
		* Called from SelectedObjectsSvc, inbound new selections
		*/
		void NodeLayer::SelectedObjectsUpdate()
		{
			if (!_bSupressSelectedObjectUpdates)
			{
				::std::vector<::NM::ODB::OBJECTUID> selectedObjects;
				_selectedObjectsSvc->GetSelected(selectedObjects);
				//SyncSelected(selectedObjects);

				// deselect current
				for each (::NM::ODB::OBJECTUID objectUID in _nodesSelected)
				{
					PNODE pNode = GetNode(objectUID);
					if (pNode)
						pNode->SetSelectedState(NodeSelectedState::Unselected);
				}
				_nodesSelected.clear();


				// copy new selected
				for each (::NM::ODB::OBJECTUID objectUID in selectedObjects)
				{
					PNODE pNode = GetNode(objectUID);
					if (pNode)
					{
						pNode->SetSelectedState(NodeSelectedState::Selected);
						_nodesSelected.push_back(objectUID);
					}
				}
			}

			// redraw
			::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget;
			GetRenderTarget(pRenderTarget);
			OnRender(pRenderTarget, true);
			Present();

			return;
		}
		/**
		* GetSelectedNodes
		*
		* From local cache
		*/
		void NodeLayer::GetSelectedNodes(NodeLayer::SELECTEDNODES& selectedNodes)
		{
			selectedNodes = _nodesSelected;
		}
		/**
		* DeselectNodes
		*
		* List of nodes to deselect
		*/
		void NodeLayer::DeselectNodes(SELECTEDNODES& selectedNodes)
		{
			// remove from internal list and set node instances to unselected
			for each (::NM::ODB::OBJECTUID uid in selectedNodes)
			{
				::std::vector<::NM::ODB::OBJECTUID>::iterator it = ::std::find(_nodesSelected.begin(), _nodesSelected.end(), uid);
				if (it != _nodesSelected.end())
				{
					_nodesSelected.erase(it);
				}

				PNODE node = GetNode(uid);
				if (node)
				{
					node->SetSelectedState(NodeSelectedState::Unselected);
				}
			}

			// dont run updates recieved from the selected objects service as we
			// sent the update and have updated our local anyway, but on update call, we redraw so no need to call here
			_bSupressSelectedObjectUpdates = true;
			// deselect globally
			_selectedObjectsSvc->Deselect(selectedNodes);
			//
			_bSupressSelectedObjectUpdates = false;

			return;
		}
		/**
		* SetSelectedNodes
		*
		* Set selected in global service and local cache
		*/
		void NodeLayer::SelectNodes(NodeLayer::SELECTEDNODES& selectedNodes)
		{
			// sync/update internal selected list, returns true if updates were made
			if (!SyncSelected(selectedNodes))
				return;
			
			::std::vector<::NM::ODB::OBJECTUID> globalSelected;
			// only get global selected is ctrl key is pressed, if not we overwrite existing selections
			if (IsControlKeyPressed())
			{
				_selectedObjectsSvc->GetSelected(globalSelected);
			}

			// copy each passed object into the global list
			for each (::NM::ODB::OBJECTUID uid in selectedNodes)
			{
				// add the new internal selected link to the global list
				globalSelected.push_back(uid);
			}
			
			// dont run updates recieved from the selected objects service as we
			// sent the update and have updated our local anyway., but on update call, we redraw so no need to call here
			_bSupressSelectedObjectUpdates = true;
			// and send new global list back to the selected service,
			// will include existing if ctrl was pressed
			_selectedObjectsSvc->Select(globalSelected);
			//
			_bSupressSelectedObjectUpdates = false;

			return;
		}
		/**
		* SyncSelected
		*
		* Syncs internal state with given list only - does not deal with global select
		* Copy from new vector of UID's to local cache vector (prob could be more intelligent, but hey ho
		* Returns true if updates were made
		*/
		bool NodeLayer::SyncSelected(SELECTEDNODES& selectedNodes)
		{
			// check to see if this is an actual update, 
			// if the passed selectedNodes is the same as the current selected dont do anything
			if (_nodesSelected == selectedNodes)
				return false;


			// lists are not the same so do updates
			if (!IsControlKeyPressed())
			{
				// deselect all existing internal selections
				for each(::NM::ODB::OBJECTUID uid in _nodesSelected)
				{
					PNODE node = GetNode(uid);
					if (!node)
						continue;
					node->SetSelectedState(NodeSelectedState::Unselected);
				}
				// clear internal selections
				_nodesSelected.clear();
			}

			// select all new objects passed to this method, set state and add to internal list of selected
			for each(::NM::ODB::OBJECTUID uid in selectedNodes)
			{
				PNODE node = GetNode(uid);
				if (!node)
					continue;
				// set node selected state
				node->SetSelectedState(NodeSelectedState::Selected);
				// add to internal selected list
				_nodesSelected.push_back(uid);
			}

			// redraw
			//OnRender(_pRenderTarget, true);
			//Present();
			return true;
		}
		/**
		* ClearAllSelectedNodes
		*
		* Clear all node selections and update Selected Objects Svc
		*/
		void NodeLayer::ClearAllSelectedNodes()
		{
			for each(::NM::ODB::OBJECTUID uid in _nodesSelected)
			{
				PNODE node = GetNode(uid);
				
				if(node)
					node->SetSelectedState(NodeSelectedState::Unselected);
			}

			_nodesSelected.clear();
			_selectedObjectsSvc->Clear();
			OnRender(_pRenderTarget, true);
		}
		/**
		*
		*
		*
		*/
		bool NodeLayer::IsSelected(::NM::ODB::OBJECTUID objectUID)
		{
			::std::vector<::NM::ODB::OBJECTUID>::iterator it = ::std::find(_nodesSelected.begin(), _nodesSelected.end(), objectUID);
			return ( it != _nodesSelected.end() ? true : false);
		}
		/**
		* DeleteNode
		*
		* Delete a node from out local cache, remove from other local caches too. 
		* Generally called on delete notify from DB Update
		*/
		void NodeLayer::DeleteNode(::NM::ODB::OBJECTUID objectUID)
		{
			// Spaital Hash
			DeleteSpatialHashNode(objectUID);

			// Node Render Order
			NODE_RENDER_ORDER::iterator renderit = ::std::find(_nodeRenderOrder.begin(), _nodeRenderOrder.end(), objectUID);
			if (renderit != _nodeRenderOrder.end())
				_nodeRenderOrder.erase(renderit);

			// Node Cache
			UID_NODE_MAP::iterator mapit = _nodeUIDLookup.find(objectUID);
			if (mapit != _nodeUIDLookup.end())
			{
				mapit->second.reset();
				//mapit->second = nullptr;
				_nodeUIDLookup.erase(mapit);
			}
			
			return;
		}
		/**
		* DatabaseUpdate
		*
		* Called from Database cache when update recieved for db updates we are interested in.
		*/
		void NodeLayer::DatabaseUpdate()
		{
			size_t qSize = _updateQ->QueueSize();
			while (!_updateQ->Empty())
			{
				::std::shared_ptr<::NM::ODB::DatabaseUpdateRecord> record = _updateQ->GetNextRecord();
				::std::wstring attrname = record->GetObjectAttributeName();
				::NM::ODB::Value val = record->GetNewValue();

				if (attrname == L"create")
				{
					assert(false);
				}
				else if (attrname == L"delete")
				{
					DeleteNode(record->GetObjectUID());
				}
				else if (attrname == L"zpos")
				{
					assert(false);
				}
				else if (attrname == L"inservice")
				{
					// existing node update
					UID_NODE_MAP::iterator nodeit = _nodeUIDLookup.find(record->GetObjectUID());
					if (nodeit != _nodeUIDLookup.end())
					{
						nodeit->second->SetAdminState((val.Get<::NM::ODB::ODBBool>() ? NodeAdminState::Enabled : NodeAdminState::Disabled));
					}
				}
				else
				{
					// existing node update - unhandled / non-specific update
					UID_NODE_MAP::iterator nodeit = _nodeUIDLookup.find(record->GetObjectUID());
					if (nodeit != _nodeUIDLookup.end())
					{
						nodeit->second->UpdateValues(attrname, val);
						if ((attrname == L"xpos") || (attrname == L"ypos"))
						{
							UpdateSpatialHashNode(record->GetObjectUID());
						}
						//++nodeit;
					}
				}
			}

			// render...
			OnRender(_pRenderTarget, true);
			Present();
			return;
		}
		/**
		* ProcessMessage
		*
		* Message called (recieved from the message pipeline) from the main window.
		* Each meesage should return 0 if we dealt with it, otherwise non-zero (1)
		* Returning 0 should be carefully considered as this may prevent layers underneath 
		* this one NOT recieving the message.
		* A method returns zero if it processes this message.
		*/
		LRESULT NodeLayer::ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam)
		{
			LRESULT lResult = 1;			
			bool redraw = false;

			switch (message)
			{
			case NVLM_GROUPDISPLAYCHANGE:
				_currentLayerUID = *((GUID*)wParam);
			case WM_SIZE:
				{					
				::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pTarget;
				GetRenderTarget(pTarget);
				RefreshNodeContent(pTarget);
				///>New
				OnRender(pTarget, true);
				lResult = 1;	// other layers need this although the parent window ignores this on WM_SIZE
				}
				break;

			case WM_LBUTTONDOWN: // lParam (POINT) already converted to client hwnd points
				lResult = _currentState->ProcessMessage(message, wParam, lParam, this, nullptr, redraw);
				break;

			case WM_LBUTTONUP:
				if (!_bSupressButtonUpProcessing)
				{
					lResult = _currentState->ProcessMessage(message, wParam, lParam, this, nullptr, redraw);
				}
				else
				{
					_bSupressButtonUpProcessing = false;
					lResult = 0; // we still processed this... 
				}
				break;

			case WM_LBUTTONDBLCLK: // lParam (POINT) already converted to client hwnd points
			{
				POINT p = { 0 };
				p.x = GET_X_LPARAM(lParam);
				p.y = GET_Y_LPARAM(lParam);
				::NM::ODB::OBJECTUID uid = IsCaretOnObject(p);
				if ((uid != ::NM::ODB::INVALID_OBJECT_UID))
				{
					// if we have dblclicked a group node, then change to that group via sendmessage to all layers
					// via parent window
					if (_odb->GetObjectTypeName(uid) == L"group")
					{
						// supress the button up processing from the dblclick as by the time
						// the message is recieved we will be on a new active layer and there will 
						// be no node underneath the mouse pointer and buttonup will return not processed 
						// and the message will be passed to lower layers which may select an object we didnt
						// intend to select.
						_bSupressButtonUpProcessing = true;
						SendLayerMessage(NVLM_GROUPDISPLAYSET, (WPARAM)&uid, 0);
						return 0;
					}
					else
					{
						lResult = _currentState->ProcessMessage(message, wParam, lParam, this, nullptr, redraw);
					}
				}
			}
			break;

			case WM_MOUSEHOVER:
			{
				POINT p = { 0 };
				p.x = GET_X_LPARAM(lParam);
				p.y = GET_Y_LPARAM(lParam);
				::NM::ODB::OBJECTUID uid = IsCaretOnObject(p);
				if (uid != ::NM::ODB::INVALID_OBJECT_UID)
				{
					SetToolTipTitle(_odb->GetValue(uid, L"shortname")->Get<::NM::ODB::ODBWString>());

					//::NM::ODB::IAdjacencyMatrix adj = _odb->GetAdjacencyMatrix();
					size_t count = _adjm->GetVertexDegree(uid);
					::std::vector<::NM::ODB::OBJECTUID> connected;
					_adjm->GetVertexConnectedVerticies(uid, connected);

					::std::wstring connectedText = L"Connected Nodes";
					for each (OBJECTUID objectUID in connected)
					{
						
						connectedText += L"\r\n" + _odb->GetValue(objectUID, L"shortname")->Get<::NM::ODB::ODBWString>();
					}


					SetToolTipDescription(::std::wstring(::std::to_wstring(count) + L" " + connectedText));
					return 0;
				}
				return 1;
			}
			break;


			case WM_MOUSEMOVE:
				lResult = _currentState->ProcessMessage(message, wParam, lParam, this, _pRenderTarget, redraw);
				break;

			case WM_RBUTTONDOWN:
				lResult = _currentState->ProcessMessage(message, wParam, lParam, this, _pRenderTarget, redraw);
				break;

			case WM_RBUTTONUP:
				lResult = _currentState->ProcessMessage(message, wParam, lParam, this, _pRenderTarget, redraw);
				break;

			case WM_KEYDOWN:
				lResult = _currentState->ProcessMessage(message, wParam, lParam, this, _pRenderTarget, redraw);
				break;

			default:
				lResult = 1;
				break;
			}


			if (redraw)
			{
				OnRender(_pRenderTarget, false); 
				Present();
			}

			return lResult;
		}
		/**
		* ProcessCommandMessage
		*
		* Any command messages from Menu buttons or popupmenus etc
		*
		* Return TRUE if message was processed
		*/
		BOOL NodeLayer::ProcessCommandMessage(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
		{
			BOOL bProcessed = FALSE;
			CCmdUI* pCmdUI = (CCmdUI*)pExtra;

			switch (nCode)
			{
			case CN_UPDATE_COMMAND_UI:
				{
					switch (nID)
					{
					// Disable/Enable 
					case 35100: 
						pCmdUI->Enable(TRUE);
						bProcessed = TRUE;
						break;

					// delete node
					case 35101:
						pCmdUI->Enable(TRUE);
						bProcessed = TRUE;
						break;
					}

				} // switch (nID)
				break;

			case CN_COMMAND:
				{
					switch (nID)
					{
					// Disable/Enable 
					case 35100:		
					{
						PNODE pNode = GetNode(_nodesSelected[0]);
						if (pNode)
						{
							if (pNode->GetAdminState() == NodeAdminState::Enabled)
								_odb->SetValue(pNode->GetNodeUID(), L"inservice", real_bool(false));
							else
								_odb->SetValue(pNode->GetNodeUID(), L"inservice", real_bool(true));
							bProcessed = TRUE;
						}
					}
					break;

					// delete node
					case 35101:
					{
						PNODE pNode = GetNode(_nodesSelected[0]);
						if (pNode)
							_odb->DeleteObject(pNode->GetNodeUID());
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
		*
		*/
		void NodeLayer::SetState(NodeLayer::LayerState newState)
		{
			::std::map<LayerState, NodeLayerState*>::iterator it = _states.find(newState);
			if (it != _states.end())
			{
#ifdef _DEBUG
				switch (newState)
				{
				case LayerState::Idle:
					OutputDebugString(L"\nSTATE CHANGE: \tChanging state to Idle");
					break;
				case LayerState::Select:
					OutputDebugString(L"\nSTATE CHANGE: \tChanging state to Select");
					break;

				case LayerState::Drag:
					OutputDebugString(L"\nSTATE CHANGE: \tChanging state to Drag");
					break;

				case LayerState::ContextMenu:
					assert(false);
					OutputDebugString(L"\nSTATE CHANGE: \tChanging state to ContextMenu");
					break;

				default:
					assert(false);
					break;
				}
#endif	
				_currentState = it->second;
			}
		}
		
		::NM::ODB::OBJECTUID NodeLayer::GetCurrentLayerUID()
		{
			return _currentLayerUID;
		}

		// ns
	}
}

