#include "stdafx.h"
#include "Sibling.h"
#include "..\DataObjects\DatabaseObject.h"
#include "..\Tables\Table.h"

namespace NM 
{
	namespace ODB
	{

		/**
		*
		*
		*
		*/
		Sibling::Sibling(Table *pTable):
			_table(pTable),
			_pFirstObject(nullptr),
			_pLastObject(nullptr)
		{
		}
		/**
		*
		*
		*
		*/
		Sibling::~Sibling(void)
		{
		}
		/**
		* AddSibling
		*
		* Adds a sibling to the end of the chain and updates the nextsibling value of the previous node to point to the new node if this is not the first node.
		*
		*/
		void Sibling::AddSibling(DatabaseObject *graphObject)
		{
			if(graphObject == nullptr)	return;

			Value goID(*graphObject->GetValue(L"uid"));
				

			// if this is the first object created
			if(_pFirstObject == nullptr)
			{
				// first object in chain
				_pFirstObject = graphObject; //newObjectUID;
				// and the last as well as its the first and only object in the chain
				_pLastObject = graphObject;  //newObjectUID;
				// set 'previoussibling' as INVALID_OBJECT_UID as this object is first in chain
				_table->SetValue( goID.Get<ODBUID>()   , L"previoussibling",  real_uid(INVALID_OBJECT_UID));		
			}
			else  // not the first object,
			{
				//Value lastID(*_pLastObject->GetValue(L"uid"));
				Value vLast( *( _pLastObject->GetValue(L"uid")) );
				OBJECTUID lastObjectUID = vLast.Get<OBJECTUID>();

				// set the new objects 'previoussibling' from _pLastObject
				_table->SetValue(goID.Get<ODBUID>(), L"previoussibling", real_uid(lastObjectUID));
				// set the old _last objects (_pLastObject ) 'nextsibling' as the new object UID

				Value vNew( *( graphObject->GetValue(L"uid")) );
				OBJECTUID newObjectUID = vNew.Get<OBJECTUID>();

				_table->SetValue(lastObjectUID, L"nextsibling", real_uid(newObjectUID));
				// set the _pLastObject as the new ObjectUID 
				_pLastObject = graphObject;
			}
	
			// set 'nextsibling' on new Object as INVALID_OBJECT_UID as this is the last object created and is end of the chain
			_table->SetValue(goID.Get<ODBUID>(), L"nextsibling", real_uid(INVALID_OBJECT_UID) );

			return;
		}
		/**
		* DeleteSibling
		*
		* Updates the previoussibling and nextsibling values of the previous and next nodes of the deleted node. 
		* Removes the deleted node from the chain and reconnects the chain where the gap is left from deleting the node
		*
		*/
		void Sibling::DeleteSibling(DatabaseObject *graphObject)
		{
			if(graphObject == nullptr) return;

			// get the (tobe) deleted objects prev and next siblings object ptrs
			Value vPrev( *( graphObject->GetValue(L"previoussibling")) );
			OBJECTUID previousSibling = vPrev.Get<OBJECTUID>();
			DatabaseObject* pPreviousSibling = _table->GetObjectPtr(previousSibling);
			
			Value vNext( *( graphObject->GetValue(L"nextsibling")) );
			OBJECTUID nextSibling = vNext.Get<OBJECTUID>();
			DatabaseObject* pNextSibling = _table->GetObjectPtr(nextSibling);
			

			// if this object was the first one, update the _pFirstObject to the new first (nextSibling)
			if(previousSibling == INVALID_OBJECT_UID)
			{
				_pFirstObject = pNextSibling;
			}
			else
			{
				// set the 'nextsibling' value of the object pointed to as 'previoussibling' by the deleted object to the 'nextsibling' value of the deleted object
				_table->SetValue(previousSibling, L"nextsibling", real_uid(nextSibling));
			}

			// if this object was the last one, update the _pLastObject to the new last (previousSibling)
			if(nextSibling == INVALID_OBJECT_UID)
			{
				_pLastObject = pPreviousSibling;
			}
			else
			{
				// set the 'previoussibling' value of the object pointed to as 'nextsibling' by the deleted object to the 'previoussibling' value of the deleted object 
				_table->SetValue(nextSibling, L"previoussibling", real_uid(previousSibling));
			}

			return;
		}

		DatabaseObject* Sibling::GetSibling(SiblingPosition position)
		{
			switch(position)
			{
			case SiblingPosition::First:
				return _pFirstObject;
				break;

			case SiblingPosition::Last:
				return _pLastObject;
				break;
			}
			return nullptr;
		}


// ns
	}
}