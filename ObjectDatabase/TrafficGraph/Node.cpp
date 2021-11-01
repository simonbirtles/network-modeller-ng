#include "stdafx.h"
#include "Node.h"

namespace NM 
{
	namespace ODB 
	{
		/**
		*
		*
		*/
		Node::Node(OBJECTUID uID, ObjectType ObjectType):
			_objectUID(uID),
			_ObjectType(ObjectType)
		{
			//OutputDebugString(L"\n****************** Node Ctor Called  ********************");
		}
		/**
		*
		*
		*/
		Node::Node(Node const &rhs)				// copy ctor
		{
			_parent = rhs._parent;
			_children = rhs._children;
			_objectUID = rhs._objectUID;
			_ObjectType = rhs._ObjectType;
		}
		/**
		*
		*
		*/
		Node& Node::operator=(Node const &rhs)			// assignment operator
		{
			_parent = rhs._parent;
			_children = rhs._children;
			_objectUID = rhs._objectUID;
			_ObjectType = rhs._ObjectType;
			return *this;
		}
		/**
		*
		*
		*/
		Node::~Node()
		{
		//	OutputDebugString(L"\n****************** Node Dtor Called  ********************");
			// for each of our children
			// 1. delete them if we are the only parent
			// 2. if more than one parent just remove us from the parent list of the child and dont delete
			CHILD_VECTOR::iterator ourChildIT = _children.begin();
			while( ourChildIT != _children.end() )
			{

				// find ourself in our childs "parent vector list"
				PARENT_VECTOR::iterator childsParentIt = ::std::find(
						(*ourChildIT)->_parent.begin(),
						(*ourChildIT)->_parent.end(),
						this);
				
				// if we found ourself
				if( childsParentIt != (*ourChildIT)->_parent.end() )
				{
					// found ourself as a parent of our child (obviously - well at least it should be)
					if( (*ourChildIT)->_parent.size() > 1)
					{						
						// as we were not only parent for this child we dont delete the child node (dtor)
						// we will just remove ourself as a parent in the child node "parent vector"
						*childsParentIt = nullptr;						// set the pointer to us as null before deleting otherwise our dtor will be called
						(*ourChildIT)->_parent.erase(childsParentIt);	// remove empty nullptr entry					
						// and then set the node in our child list to nullptr, as we in dtor no point in deleting entry
						*ourChildIT = nullptr;
					}
					else if( (*ourChildIT)->_parent.size() == 1)
					{
						// we will just remove ourself as a parent in the child node "parent vector"
						*childsParentIt = nullptr;						// set the pointer to us as null before deleting otherwise our dtor will be called
						(*ourChildIT)->_parent.erase(childsParentIt);	// remove empty nullptr entry	

						//// if the child node is a Path, we dont delete these automatically, TrafficGraph will clean up any orphan ones.
						//if( (*ourChildIT)->GetObjectType() != ObjectType::Path )
						//{
							// only us as parents left, delete the child node by calling delete on the child ptr in our child vector
							delete *ourChildIT;
							*ourChildIT = nullptr;
						//}
					}		
				}		
				++ourChildIT;
			}

			
			// for each of our parents....
			PARENT_VECTOR::iterator ourParentIt = _parent.begin();
			while( ourParentIt != _parent.end() )
			{
				// find ourself in our parents child vector list 
				CHILD_VECTOR::iterator parentChildit = ::std::find(
						(*ourParentIt)->_children.begin(),
						(*ourParentIt)->_children.end(),
						this);
				
				// if we find ourself as a child in our parents "child vector list"
				if( parentChildit != (*ourParentIt)->_children.end())
				{
					// set ptr to us to nullptr, dont just erase element just yet or it will call this dtor we are currently executing
					*parentChildit = nullptr;
					// now erase ptr to null from parents child vector list
					(*ourParentIt)->_children.erase(parentChildit);
				}
				++ourParentIt;
			}
		}
		/**
		*
		*
		*/
		bool Node::HasParents()
		{
			return ( _parent.size() != 0 );
		}
		/**
		*
		*
		*/
		size_t Node::GetParentCount()
		{
			return _parent.size();
		}
		/**
		*
		*
		*/
		Node* Node::GetParent(size_t parentIndex)
		{
			return _parent[parentIndex];
		}
		/**
		*
		*
		*/
		Node* Node::AddChild(OBJECTUID uID, ObjectType ObjectType)
		{
			//Node* child = ;
			// we are this childs parent, so set us as a parent in the child
			//child->_parent.push_back(this);
			// as we are a parent of this child, then save this child as our child
			_children.push_back(new Node(uID, ObjectType));
			CHILD_VECTOR::iterator it = _children.end() -1;
			(*it)->_parent.push_back(this);


			return (*it);
		}
		/**
		*
		*
		*/
		size_t Node::GetChildCount()
		{
			return _children.size();
		}
		/**
		*
		*
		*/
		Node* Node::GetChild(size_t childIndex)
		{
			return _children[childIndex];
		}
		/**
		*
		*
		*/
		OBJECTUID Node::GetObjectUID()
		{
			return _objectUID;
		}
		/**
		*
		*
		*/
		ObjectType Node::GetObjectType()
		{
			return _ObjectType;
		}

// ns
	}
}
