#pragma once
#include "..\Interfaces\ObjectDatabaseDefines.h"
#include <vector>

namespace NM 
{
	namespace ODB 
	{

		class Node
		{
		public:


			Node(OBJECTUID uID, ObjectType ObjectType);
			Node(Node const &rhs);				// copy ctor
			Node& operator=(Node const &rhs);			// assignment operator
			~Node();

			bool HasParents();
			size_t GetParentCount();
			Node* GetParent(size_t parentIndex);
			
			Node* AddChild(OBJECTUID uID, ObjectType ObjectType);
			size_t GetChildCount();
			Node*	GetChild(size_t childIndex);
				
			OBJECTUID GetObjectUID();
			ObjectType GetObjectType();
			

		private:
			typedef ::std::vector<Node*> PARENT_VECTOR;
			typedef ::std::vector<Node*> CHILD_VECTOR;			
			PARENT_VECTOR _parent;			
			CHILD_VECTOR _children;
			OBJECTUID	_objectUID;
			ObjectType	_ObjectType;
			
		};


	}
}
