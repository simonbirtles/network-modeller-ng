#pragma once
#include "Interfaces\ObjectDatabaseDefines.h"
#include <memory>

namespace NM
{
	namespace ODB
	{
		class GroupNode
		{
		public:
			typedef GroupNode* PNODE;

			GroupNode();
			~GroupNode();
			void Init(::NM::ODB::OBJECTUID groupUID, PNODE pParentNode);
			::NM::ODB::OBJECTUID GetNodeUID();
			bool HasChildren() { return _pFirstChild != nullptr; };
			void DeleteTreeDFS();

		private:
			PNODE					_pParent;
			::NM::ODB::OBJECTUID	_groupUID;
			PNODE					_pNextSibling;
			PNODE					_pFirstChild;
		};

	}
}

