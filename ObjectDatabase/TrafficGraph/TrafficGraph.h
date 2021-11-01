#pragma once
#include "..\Interfaces\ObjectDatabaseDefines.h"
//#include "..\DatabaseCoreElements\internal.h"

#include <map>

namespace NM 
{
	namespace ODB 
	{
		class Node;

		class TrafficGraph
		{
		public:
			TrafficGraph();
			~TrafficGraph();

			bool		AddNode(OBJECTUID parent, OBJECTUID uid, ObjectType ObjectType);
			bool		DeleteNode(OBJECTUID uid);
		
			OBJECTUID	GetRootNode();
			
			size_t		GetParentCount(OBJECTUID uid);
			OBJECTUID	GetParent(OBJECTUID uid, size_t index);

			size_t		GetChildCount(OBJECTUID uid);
			OBJECTUID	GetChild(OBJECTUID uid, size_t index);

			ObjectType	GetObjectType(OBJECTUID uid);
			bool		IsInGraph(OBJECTUID uid);
			void		Clear();

		private:
			Node* _rootNode;
			typedef ::std::map<OBJECTUID, Node*, compareGuid> UIDPNODE;
			UIDPNODE _uid_to_pnode;

			bool DeleteNodeDFS(OBJECTUID uid);

		};


	}
}
