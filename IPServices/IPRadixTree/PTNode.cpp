#include "stdafx.h"
#include "PTNode.h"
#include <assert.h>

namespace NM
{
	namespace IPRTREE
	{


		/**
		*
		*
		*
		*/
		PTNode::PTNode() :
			_left(nullptr),
			_right(nullptr),
			_bitIndex(-1),			// -1 would be root
			_prefixLength(0),
			_pInterfaceList(nullptr)
		{
		}
		/**
		*
		*
		*
		*/
		PTNode::~PTNode()
		{
			if (_pInterfaceList) delete _pInterfaceList;
			assert(_left == nullptr);
			assert(_right == nullptr);
		}


	}
}