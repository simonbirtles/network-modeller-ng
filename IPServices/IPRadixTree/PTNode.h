#pragma once
#include <set>
#include <Rpc.h>
#include <cguid.h>
#include "IPRadixDef.h"
/**
* class PTNode
*
*	If:
*	Interface.size()>0 and prefixlength>0, then this is a interface assigned network route or host/route, _left and/or _right can both be null or not.
*	Interface.size()>0 and prefixlength==0, then this is an ERROR
*	Interface.size()=0 and prefixlength>0, then this is a network organisational summary prefixand and _left or _right should not be null, otherwise ERROR
*	Interface.size()=0 and prefixlength==0, then this is a transitive node which is part of a stored prefix and _left or _right should not be null, otherwise ERROR
*/

namespace NM
{
	namespace IPRTREE
	{
		
		class PTNode
		{
			//typedef GUID OBJECTUID;
			
		public:
			friend class PTree;

			PTNode();
			~PTNode();

		private:
			// ordered for 32/64 bit alignment
			PTNode*					_left;				// bit set 0
			PTNode*					_right;				// bit set 1
			INTFLIST*				_pInterfaceList;	// if not null, list containing interfaces that this prefix is assigned to or interfaces next hop where this prefix is advertised into the AS from.
			//::NM::ODB::OBJECTUID	_interfaceUID;		// interface UIDs, the interfaces that this prefix is assigned to, zero, one or more.
			unsigned __int8			_prefixLength;		// prefix length, much like bitindex minus RD length, but when not 0 it is used to indicate that this prefix is required when ip find/enumeration is done, does not require any interfaces as it could be a organisation prefix
			__int16					_bitIndex;			// the bit index represents the depth this node is in the tree and includes the RD depth too.

		};

	}
}

