#pragma once
#include "BaseLayer.h"
#include "Interfaces\ObjectDatabaseDefines.h"
#include "Interfaces\IDatabaseUpdate.h"					// for client interface 
#include "Interfaces\DatabaseObserver.h"					// base class for client listener (dervive from for update calls)
#include "SelectedObjectsClientObs.h"
//#include "NodeLayerState.h"


#include <vector>
#include <memory>
#include <map>
#include <unordered_map>
#include <set>
#include <wrl.h>

namespace NM
{
	class ISelectedObjects;

	namespace ODB
	{
		class IObjectDatabase;
		class IDatabaseUpdate;
	}

	namespace DataServices
	{
		class IAdjacencyMatrix;
	}

	namespace NetGraph
	{
		class Node;
		class BitmapCache;
		class NodeLayerState;
		class NodeDraggingState;
		class NetworkViewSpatialHash;


		class NodeLayer : 
			public BaseLayer, 
			public ::NM::ODB::CDatabaseObserver,
			public ::NM::SelectedObjectsClientObs
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

			enum class LayerState { Idle, Select, Drag, ContextMenu };
			typedef ::std::vector<::NM::ODB::OBJECTUID> SELECTEDNODES;

			typedef ::std::shared_ptr<Node> PNODE;

			NodeLayer();
			~NodeLayer();

			// Parent calls into us (from NetworkView)
			HRESULT OnRender(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> &pRenderTarget, bool bRedraw);
			bool ShouldRender() { return true; };
			LRESULT ProcessMessage(UINT, WPARAM, LPARAM);
			BOOL	ProcessCommandMessage(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
			bool Initialise();
			::std::wstring GetLayerText() { return L"Node Layer"; }

			// from DatabaseUpdateCache
			void DatabaseUpdate();
			// From SelectedObjectService
			void SelectedObjectsUpdate();
			void MultiSelectChange(bool bMultiSelect);


			// used by LayerStates
			bool IsSelected(::NM::ODB::OBJECTUID);
			void GetSelectedNodes(SELECTEDNODES&);
			void SelectNodes(SELECTEDNODES&);
			void DeselectNodes(SELECTEDNODES&);
			void ClearAllSelectedNodes();
			::NM::ODB::OBJECTUID GetCurrentLayerUID();
			
			::std::shared_ptr<BitmapCache> GetBitmapCache() { return _bmpCache; };
			PNODE CopyNode(::NM::ODB::OBJECTUID);
			PNODE CreateNewNode(::NM::ODB::OBJECTUID);
			PNODE GetNode(::NM::ODB::OBJECTUID);
			::NM::ODB::OBJECTUID IsCaretOnObject(POINT &clientWindowPoint);
			void SetState(LayerState);
			HWND GetHWND() { return BaseLayer::GetHWND(); };

			POINT _lastMouseDownPoint;
			

		private:
			bool SyncSelected(SELECTEDNODES&);

		// used to draw the later text - for testing
			IDWriteFactory* pDWriteFactory_;
			::Microsoft::WRL::ComPtr<IDWriteTextFormat> pTextFormat_;
			const wchar_t* wszText_;
			UINT32 cTextLength_;
			::Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> pTextWhiteBrush_;
		// end testing draw layer text 

			::std::unique_ptr<::NM::DataServices::IAdjacencyMatrix>		_adjm;
			::std::unique_ptr<::NM::ODB::IObjectDatabase>				_odb;					// interface to the database
			::std::unique_ptr<NM::ODB::IDatabaseUpdate>					_updateCache;			// interface to the db update cache
			::NM::ODB::IDatabaseUpdate::UpdateQueueHandle				_updateQ;				// out personal queue of updates to db we requested
			::std::unique_ptr<NM::ISelectedObjects>						_selectedObjectsSvc;
			::std::shared_ptr<::NM::NetGraph::NetworkViewSpatialHash>	_spatialHash;
			::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget>			_pRenderTarget;			// the last render target passed to us
			::Microsoft::WRL::ComPtr<ID2D1Bitmap>						_pBitmap;				// static layer content bitmap cache
			::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget>			_pStaticBitmap;			// static layer content bitmap cache
			::NM::ODB::real_uid											_currentLayerUID;

			// states
			NodeLayerState* _currentState;
			::std::map<LayerState, NodeLayerState*> _states;

			// UID<>Node/Bitmap caches
			typedef ::std::map<::NM::ODB::OBJECTUID, PNODE, compareGuid>	UID_NODE_MAP;
			typedef ::std::vector<::NM::ODB::OBJECTUID>						NODE_RENDER_ORDER;
			NODE_RENDER_ORDER	_nodeRenderOrder;											// node draw order (zpos)
			UID_NODE_MAP		_nodeUIDLookup;												// UID -> Node* 
			::std::shared_ptr<BitmapCache> _bmpCache;										// Node Bitmap Cache (used by nodes directly)

			// methods
			void RenderNodes(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget);
			void RefreshNodes(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget);
			void RefreshNodeContent(::Microsoft::WRL::ComPtr<ID2D1BitmapRenderTarget> pRenderTarget);
			void DeleteLocalNodeCache();
			void DeleteNode(::NM::ODB::OBJECTUID);
			
			// Spatial Hash
			typedef std::pair<int, int> CELLXY;														// Cell reference e.g (1,3)
			void InsertSpatialHashNode(::NM::ODB::OBJECTUID);
			void UpdateSpatialHashNode(::NM::ODB::OBJECTUID);
			void DeleteSpatialHashNode(::NM::ODB::OBJECTUID);
			int _spatialGridSize;

			// state flags
			bool _leftButtonDown;
			bool _dragging;
			bool _bSupressButtonUpProcessing;
			bool _bSupressSelectedObjectUpdates;

			// mouse point
			POINT _lastMousePoint;

			// state structures
			SELECTEDNODES _nodesSelected;
		};

		// ns
	}
}

