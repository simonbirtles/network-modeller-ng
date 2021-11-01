#include "stdafx.h"
#include "GroupNode.h"

namespace NM
{
	namespace ODB
	{
		/**
		*
		*
		*
		*/
		GroupNode::GroupNode() 			
		{			
		}
		/**
		*
		*
		*
		*/
		GroupNode::~GroupNode()
		{
			// first check to see if we are the root, if so dont do anything
			if (_groupUID != ::NM::ODB::INVALID_OBJECT_UID)
			{
				// make sure we dont have any children left
				assert(_pFirstChild == nullptr);
				// check our siblings and adjust siblings linkedlist if we have siblings
				
				// are we the parents first child ?
				assert(_pParent->_pFirstChild != nullptr);	// check valid.. shoudl be
				// are we the parents first child
				if (_pParent->_pFirstChild == this)
				{
					// set our parent first child to our first sibling, fine if null 
					// as means our parent has no children anymore.
					_pParent->_pFirstChild = _pNextSibling;

				}
				// nope we are not the first child so find us in the sibling linkedlist
				else
				{
					PNODE pOurPreviousSibling = _pParent->_pFirstChild; // the first of our siblings
					while (pOurPreviousSibling->_pNextSibling != this)
					{
						pOurPreviousSibling = pOurPreviousSibling->_pNextSibling;
						assert(pOurPreviousSibling != nullptr);	// just check we dont run past end point
					}

					pOurPreviousSibling->_pNextSibling = this->_pNextSibling;
				}
			}
		}
		/**
		*
		*
		*
		*/
		/**
		* DeleteTreeDFS
		*
		* Iterates through the tree from the given start node using a DFS walk
		*/
		void GroupNode::DeleteTreeDFS()
		{
			// any children, do them first
			if (_pFirstChild)
			{
				_pFirstChild->DeleteTreeDFS();
				delete _pFirstChild;
				_pFirstChild = nullptr;
			}

			// any siblings, do them next
			if (_pNextSibling)
			{
				_pNextSibling->DeleteTreeDFS();
				delete _pNextSibling;
				_pNextSibling = nullptr;
			}

			return;
		}
		/**
		*
		*
		*
		*/
		void GroupNode::Init(::NM::ODB::OBJECTUID groupUID, PNODE pParentNode)
		{
			_pParent = pParentNode;
			_groupUID = groupUID;

			// if we are not creating the virtual root node which has invalid group id
			if (_groupUID != ::NM::ODB::INVALID_OBJECT_UID)
			{
				// get our parents first child, then iterate through the children (siblings) 
				// until we find the last one, so we can add ourselves, unless we are first child so we add ourselves as first child
				PNODE pNode = _pParent->_pFirstChild;
				if (pNode)
				{
					while (pNode->_pNextSibling)
					{
						pNode = pNode->_pNextSibling;
					}
					// set us as last sibling
					pNode->_pNextSibling = this;
				}
				// we are first child
				else
				{
					// set us as first child of parent
					_pParent->_pFirstChild = this;
				}
			}
			else
			{
				// just ensure we have correct invalid parent setup too
				if (_pParent != nullptr)
				{
					throw ::std::logic_error("Root Node has a non NULL parent pointer");
				}
			}
		}
		/**
		*
		*
		*
		*/
		::NM::ODB::OBJECTUID GroupNode::GetNodeUID()
		{
			return _groupUID;
		}



		// ns
	}
}
