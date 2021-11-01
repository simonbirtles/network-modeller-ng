#pragma once
#include "..\interfaces\objectdatabasedefines.h"
#include "..\ObjectDatabase\Interfaces\DatabaseObserver.h"					// base class for client listener (dervive from for update calls)
#include "..\ObjectDatabase\Interfaces\IDatabaseUpdate.h"	
#include <memory>

namespace NM
{
	namespace ODB
	{

		//class TriggerManager;
		class ITrigger;
		class CObjectDatabase;
		class IDatabaseUpdate;
		class IObjectDatabase;
		class GroupNode;

		class GroupManager :
			public ::NM::ODB::CDatabaseObserver
		{
		public:
			typedef GroupNode* PNODE;

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

			GroupManager(CObjectDatabase*, IDatabaseUpdate*);
			~GroupManager();
			void DatabaseUpdate();
			bool GroupDeleteTrigger(IObjectDatabase&, OBJECTUID, const Value& pValue, const Value& nValue);
			bool GroupInsertTrigger(IObjectDatabase&, OBJECTUID, const Value& pValue, const Value& nValue);
			bool GroupUpdateTrigger(IObjectDatabase&, OBJECTUID, const Value& pValue, const Value& nValue);

		private:
			CObjectDatabase*	_odb;
			IDatabaseUpdate*	_updateCache;
			::NM::ODB::IDatabaseUpdate::UpdateQueueHandle	 _updateQ;
			::std::shared_ptr<ITrigger>		_triggerManager;
			TRIGGERID			_deleteTriggerID;
			TRIGGERID			_insertTriggerID;
			TRIGGERID			_updateTriggerID;

			void Init();
			void RefreshGroupCache();
			void BuildTree(PNODE pParent);
			void BuildNodeToGroupCache();
			void ClearCaches();
			
			PNODE GetGroupNodePointer(::NM::ODB::OBJECTUID);
			::std::vector<::NM::ODB::OBJECTUID> GetNodesInGroup(::NM::ODB::OBJECTUID);
			size_t GetNodesCountInGroup(::NM::ODB::OBJECTUID);

			typedef ::std::map<::NM::ODB::OBJECTUID, PNODE, compareGuid>								GROUPUID_NODE_MAP;
			typedef ::std::map<::NM::ODB::OBJECTUID, ::std::vector<::NM::ODB::OBJECTUID>, compareGuid>	GROUPUID_NODEUID_MAP;
			GROUPUID_NODE_MAP		_groupUidNodeMap;		// maps group uid to Group PNODE
			GROUPUID_NODEUID_MAP	_groupUidNodeUidMap;	// maps group group uid to vertex UID (what vertex's are in a group)

			::std::vector<PNODE> _nodes;
			PNODE				_rootNode;

		};

	}
}