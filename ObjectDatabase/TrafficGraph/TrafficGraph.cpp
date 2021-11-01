#include "stdafx.h"
#include "TrafficGraph.h"
#include "Node.h"

namespace NM 
{
	namespace ODB 
	{
		/**
		*
		*
		*
		*/
		TrafficGraph::TrafficGraph(void):
			_rootNode(nullptr)
		{
			UUID newUID;
			UuidCreate( &newUID );
			_rootNode = new Node(newUID, ObjectType::ObjectInvalid);
			// add the root to the lookup index
			_uid_to_pnode[newUID] = _rootNode;
		}
		/**
		*
		*
		*
		*/
		TrafficGraph::~TrafficGraph(void)
		{
			UIDPNODE::iterator it = _uid_to_pnode.find(_rootNode->GetObjectUID());
			if( it != _uid_to_pnode.end() )
			{
				delete _rootNode;
				_rootNode = nullptr;
				_uid_to_pnode.erase(it);
			}

			// check for orphans or more likely to be paths not assigned to any demand which is allowed.
			//it = _uid_to_pnode.begin();
			//while( it != _uid_to_pnode.end() )
			//{
			//	delete it->second;
			//	++it;
			//}

		}
		/**
		*
		*
		*
		*/
		bool TrafficGraph::AddNode(OBJECTUID parent, OBJECTUID uid, ObjectType ObjectType)
		{
			if( parent == INVALID_OBJECT_UID )
			{
				// use root 
				parent = _rootNode->GetObjectUID();
			}

			UIDPNODE::iterator it = _uid_to_pnode.find(parent);
			if( it == _uid_to_pnode.end() )
			{
				return false; // parent does not exist
			}

			// create new child
			//Node* child = new Node(uid, ObjectType);

			// try to add to the parent
			Node* child = it->second->AddChild(uid, ObjectType);
			if( !child )
			{
				return false;
			}

			_uid_to_pnode[uid] = child;

			return true;
		}
		/**
		*
		* Delete the node. 
		* Node will clean up parent refs on dtor. 
		* DeleteNode calls DeleteNodeDFS to clean up all children in the tree.
		*
		*/
		bool TrafficGraph::DeleteNode(OBJECTUID uid)
		{
			UIDPNODE::iterator it = _uid_to_pnode.find(uid);
			if( it == _uid_to_pnode.end() )
			{
				return false; // node does not exist
			}

			delete it->second;
			it->second = nullptr;
			_uid_to_pnode.erase(it);
				
			return true;
			//return DeleteNodeDFS(uid);		
		}
		/**
		* DeleteNodeDFS
		*
		* Not sure this is needed as when DeleteNode is called it deletes all children anyway.
		*/
		bool TrafficGraph::DeleteNodeDFS(OBJECTUID uid)
		{
			UIDPNODE::iterator it = _uid_to_pnode.find(uid);
			if( it == _uid_to_pnode.end() )
			{
				return false; // node does not exist
			}

			if( it->second->GetChildCount() )
			{
				for(size_t i=0; i< it->second->GetChildCount(); ++i)
				{
					DeleteNodeDFS( it->second->GetChild(i)->GetObjectUID() );
				}
			}

			// if this node/we has/have more than one parent, then dont delete it
			if( it->second->GetParentCount() <= 1)
			{
				// when we call delete on the Node, the node destructor will also manage removing
				// itself from its parents.
				delete it->second;
				it->second = nullptr;
				//_uid_to_pnode.erase(it);		// dont erase from map as it will invalidate recursive iterators, having the "dead" ones in the map wont hurt as long as the app is not running forever
				return true;					// true we have deleted
			}

			return false; // false we have not deleted due to >1 parents
		}
		/**
		*
		*
		*
		*/
		OBJECTUID TrafficGraph::GetRootNode()
		{
			return _rootNode->GetObjectUID();
		}
		/**
		*
		*
		*
		*/
		size_t TrafficGraph::GetChildCount(OBJECTUID uid)
		{
			UIDPNODE::iterator it = _uid_to_pnode.find(uid);
			if( it == _uid_to_pnode.end() )
			{
				return 0; // node does not exist
			}
			
			return it->second->GetChildCount();
		}
		/**
		*
		*
		*
		*/
		OBJECTUID TrafficGraph::GetChild(OBJECTUID uid, size_t index)
		{
			UIDPNODE::iterator it = _uid_to_pnode.find(uid);
			if( it == _uid_to_pnode.end() )
			{
				return INVALID_OBJECT_UID; // node does not exist
			}
			
			return (it->second->GetChild(index))->GetObjectUID();
		}
		/**
		*
		*
		*
		*/
		size_t TrafficGraph::GetParentCount(OBJECTUID uid)
		{
			UIDPNODE::iterator it = _uid_to_pnode.find(uid);
			if( it == _uid_to_pnode.end() )
			{
				return 0; // node does not exist
			}
			
			return it->second->GetParentCount();
		}
		/**
		*
		*
		*
		*/
		OBJECTUID TrafficGraph::GetParent(OBJECTUID uid, size_t index)
		{
			UIDPNODE::iterator it = _uid_to_pnode.find(uid);
			if( it == _uid_to_pnode.end() )
			{
				return INVALID_OBJECT_UID; // node does not exist
			}
			
			return (it->second->GetParent(index))->GetObjectUID();
		}
		/**
		*
		*
		*
		*/
		ObjectType TrafficGraph::GetObjectType(OBJECTUID uid)
		{
			UIDPNODE::iterator it = _uid_to_pnode.find(uid);
			if( it == _uid_to_pnode.end() )
			{
				return ObjectType::ObjectInvalid; // node does not exist
			}
			
			return it->second->GetObjectType();
		}
		/**
		*
		*
		*
		*/
		bool TrafficGraph::IsInGraph(OBJECTUID uid)
		{
			UIDPNODE::iterator it = _uid_to_pnode.find(uid);
			return ( it != _uid_to_pnode.end() );
		}
		/**
		*
		*
		*
		*/
		void TrafficGraph::Clear()
		{
			DeleteNodeDFS(_rootNode->GetObjectUID());
			_uid_to_pnode.clear();

			// create new root node
			UUID newUID;
			UuidCreate( &newUID );
			_rootNode = new Node(newUID, ObjectType::ObjectInvalid);
			// add the root to the lookup index
			_uid_to_pnode[newUID] = _rootNode;
			
		}

		// ns

	}
}