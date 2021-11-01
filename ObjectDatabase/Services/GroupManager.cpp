#include "stdafx.h"
#include "GroupManager.h"
#include "..\Database\ObjectDatabase.h"
#include "..\Database\TriggerManager.h"
#include "Interfaces\IDatabaseUpdate.h"					// client interface to database updates
#include "Interfaces\IObjectDatabase.h"
#include "Interfaces\ITrigger.h"
#include "GroupNode.h"


namespace NM
{
	namespace ODB
	{

		GroupManager::GroupManager(CObjectDatabase* odb, IDatabaseUpdate* updateCache):
			_odb(odb),
			_updateCache(updateCache),
			_deleteTriggerID(::NM::ODB::INVALID_OBJECT_UID)
		{
			Init();
		}


		GroupManager::~GroupManager()
		{
			ClearCaches();
		}
		/**
		*
		*
		*
		*/
		void GroupManager::Init()
		{
			RefreshGroupCache();

			// create require updates list
			::std::vector<::std::wstring> attr_list;
			attr_list.push_back(L"create");
			attr_list.push_back(L"delete");
			attr_list.push_back(L"group");		// groups parent change
			_updateQ = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Group, attr_list);
			_updateQ = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Vertex, attr_list);

			attr_list.clear();
			attr_list.push_back(L"*");		
			_updateQ = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Database, attr_list);
			
			// create trigger for group deletes
			_triggerManager = _odb->GetTriggerInterface();

			::std::wstring tableName = L"grouptable";
			::std::wstring attributeName = L"";		
			TriggerFunc deletetrigger = std::bind(
				&GroupManager::GroupDeleteTrigger, 
				this, 
				std::placeholders::_1, 
				std::placeholders::_2, 
				std::placeholders::_3,
				std::placeholders::_4
				);			
			_deleteTriggerID = _triggerManager->CreateTrigger(tableName, attributeName, TriggerOperation::Delete, deletetrigger);

			TriggerFunc inserttrigger = std::bind(
				&GroupManager::GroupInsertTrigger,
				this,
				std::placeholders::_1,
				std::placeholders::_2,
				std::placeholders::_3,
				std::placeholders::_4
				);
			_insertTriggerID = _triggerManager->CreateTrigger(tableName, attributeName, TriggerOperation::Insert, inserttrigger);

			attributeName = L"group";
			TriggerFunc updatetrigger = std::bind(
				&GroupManager::GroupUpdateTrigger,
				this,
				std::placeholders::_1,
				std::placeholders::_2,
				std::placeholders::_3,
				std::placeholders::_4
				);
			_updateTriggerID = _triggerManager->CreateTrigger(tableName, attributeName, TriggerOperation::Update, updatetrigger);

			return;
		}
		/**
		* GroupUpdateTrigger
		*
		* Validates the requested group update - this is for the objects "group" change
		*/
		bool GroupManager::GroupUpdateTrigger(IObjectDatabase&, OBJECTUID objectUID, const Value& pValue, const Value& nValue)
		{
			OutputDebugString(L"\nGroupManager::GroupUpdateTrigger --> Not Implemented: Update Denied..");
			return false;
		}
		/**
		* GroupCreateTrigger
		*
		* Validates the requested new group before creation
		*/
		bool GroupManager::GroupInsertTrigger(IObjectDatabase&, OBJECTUID objectUID, const Value& pValue, const Value& nValue)
		{
			::NM::ODB::OBJECTUID groupParentGroup = _odb->GetValue(objectUID, L"group")->Get<::NM::ODB::ODBUID>();
			// if parent is root group, allow..
			if (groupParentGroup == ::NM::ODB::INVALID_OBJECT_UID)
				return true; 

			// check the new groups parent group is valid
			GROUPUID_NODE_MAP::iterator it = _groupUidNodeMap.find(groupParentGroup);
			return(it != _groupUidNodeMap.end());
		}
		/**
		* GroupDeleteTrigger
		*
		* objectUID is the group UID
		* Called by database when an delete request has been recieved for a group
		* Returning true will allow the delete, false will disallow the delete.
		*/
		bool GroupManager::GroupDeleteTrigger(IObjectDatabase&, OBJECTUID objectUID, const Value& pValue, const Value& nValue)
		{
			// find the object Node instance for the objectUID
			GROUPUID_NODE_MAP::iterator it = _groupUidNodeMap.find(objectUID);
			if (it != _groupUidNodeMap.end())
			{
				if( (it->second->HasChildren()) || (GetNodesCountInGroup(objectUID) > 0)) 
				{
					return false;
				}
				return true;
			}
			// unknown object
			return false;
		}
		/**
		* ClearCaches
		*
		* Delete all local cache data including virtual root node 
		*/
		void GroupManager::ClearCaches()
		{
			if (_rootNode)
			{
				_rootNode->DeleteTreeDFS();
				delete _rootNode;
				_rootNode = nullptr;
			}

			_nodes.clear();
			_groupUidNodeMap.clear();
			return;
		}
		/**
		* BuildTree
		*
		* Recursive function to loop through all groups to build a tree
		*/
		void GroupManager::BuildTree(GroupNode::PNODE pParent)
		{
			// groups
			::std::wstring table = L"grouptable";
			::std::wstring text;
			::NM::ODB::OBJECTUID objectUID = _odb->GetFirstObject(table);
		
			//loop through all groups and for each group that has a parent "group" of the passed parent
			// create a new node with the groups UID and the parent and add to local cache
			while (objectUID != NM::ODB::INVALID_OBJECT_UID)
			{
				::NM::ODB::OBJECTUID parent = _odb->GetValue(objectUID, L"group")->Get<NM::ODB::ODBUID>();
				// if we find objects with the passed parent add these now
				if (parent == pParent->GetNodeUID())
				{
					GroupNode::PNODE pNode = new GroupNode;
					pNode->Init(objectUID, pParent);
					_nodes.push_back(pNode);
					_groupUidNodeMap[objectUID] = pNode;
					BuildTree(pNode);
				}
				objectUID = _odb->GetNextObject(objectUID);
			}
			return;
		}
		/**
		*
		*
		*
		*/
		::std::vector<::NM::ODB::OBJECTUID> GroupManager::GetNodesInGroup(::NM::ODB::OBJECTUID nodeGroupUID)
		{
			::std::vector<::NM::ODB::OBJECTUID> copy;
			// do we have a mapping for this groupid in the groupid->vector<nodeid>
			GROUPUID_NODEUID_MAP::iterator it = _groupUidNodeUidMap.find(nodeGroupUID);
			if (it != _groupUidNodeUidMap.end())
			{
				copy = it->second;
			}

			throw ::std::runtime_error("GetNodesInGroup failed to find group");
			return copy;
		}
		/**
		*
		*
		*
		*/
		size_t GroupManager::GetNodesCountInGroup(::NM::ODB::OBJECTUID nodeGroupUID)
		{
			::std::vector<::NM::ODB::OBJECTUID> copy;
			// do we have a mapping for this groupid in the groupid->vector<nodeid>
			GROUPUID_NODEUID_MAP::iterator it = _groupUidNodeUidMap.find(nodeGroupUID);
			if (it != _groupUidNodeUidMap.end())
			{
				return it->second.size();;
			}

			return 0;
		}
		/**
		* RefreshGroupCache
		*
		* Deletes all existing data and rebuilds tree from Database.
		*/
		void GroupManager::RefreshGroupCache()
		{
			// clean out existing data
			ClearCaches();

			_rootNode = (new GroupNode);
			_rootNode->Init(::NM::ODB::INVALID_OBJECT_UID, nullptr);

			// build new cache
			BuildTree(_rootNode);

			// cache db nodes to group
			BuildNodeToGroupCache();

			return;
		}
		/**
		* BuildNodeToGroupCache
		*
		* Builds cache of Group to NodeUIDs. Provides lookup to find all nodes in a group quickly.
		*/
		void GroupManager::BuildNodeToGroupCache()
		{
			::std::wstring table = L"vertextable";
			::std::wstring text;
			::NM::ODB::OBJECTUID objectUID = _odb->GetFirstObject(table);

			while (objectUID != NM::ODB::INVALID_OBJECT_UID)
			{
				// get this vertex group
				::NM::ODB::OBJECTUID nodeGroupUID = _odb->GetValue(objectUID, L"group")->Get<NM::ODB::ODBUID>();

				// if we have this group in our cache (whcih we should)
				PNODE pNode = GetGroupNodePointer(nodeGroupUID);
				assert(pNode);
				if (pNode)
				{
					// do we have a mapping for this groupid in the groupid->vector<nodeid>
					GROUPUID_NODEUID_MAP::iterator it = _groupUidNodeUidMap.find(nodeGroupUID);
					if (it != _groupUidNodeUidMap.end())
					{
						it->second.push_back(objectUID);
					}
					// no, so create new mapping with new vector
					else
					{
						::std::vector<::NM::ODB::OBJECTUID> vec;
						vec.push_back(objectUID);
						_groupUidNodeUidMap[nodeGroupUID] = vec;
					}					
				}
				objectUID = _odb->GetNextObject(objectUID);
			}
			return;
		}
		/**
		*
		*
		*
		*/
		GroupManager::PNODE GroupManager::GetGroupNodePointer(::NM::ODB::OBJECTUID objectUID)
		{
			if (objectUID == ::NM::ODB::INVALID_OBJECT_UID)
				return _rootNode;

			GROUPUID_NODE_MAP::iterator it = _groupUidNodeMap.find(objectUID);
			if (it != _groupUidNodeMap.end())
			{
				return it->second;
			}
			return nullptr;
		}
		/**
		*
		*
		*
		*/
		void GroupManager::DatabaseUpdate()
		{
			//size_t qSize = _updateQ->QueueSize();
			//OutputDebugString(L"\nGroupManager::DatabaseUpdate()");
			//OutputDebugString(L"\nQ-Size:\t");
			//OutputDebugString(::std::to_wstring(qSize).c_str());
			while (!_updateQ->Empty())
			{
				::std::shared_ptr<::NM::ODB::DatabaseUpdateRecord> record = _updateQ->GetNextRecord();
				::NM::ODB::DatabaseUpdateType updateType = record->GetUpdateType();
				::NM::ODB::OBJECTUID objectUID = record->GetObjectUID();
				switch (updateType)
				{
				case ::NM::ODB::DatabaseUpdateType::Create:
					//OutputDebugString(L"\n****DatabaseUpdateType::Create");
					{
						
						switch (record->GetObjectType())
						{

						case ::NM::ODB::ObjectType::Group:
							// already validated by us so just need to add the group to the tree
							{
								::NM::ODB::OBJECTUID objectGroupParentUID = _odb->GetValue(objectUID, L"group")->Get<::NM::ODB::ODBUID>();
								GROUPUID_NODE_MAP::iterator it = _groupUidNodeMap.find(objectGroupParentUID);
								if (it != _groupUidNodeMap.end())
								{
									GroupNode::PNODE pNode = new GroupNode;
									pNode->Init(objectUID, it->second);
									_nodes.push_back(pNode);
									_groupUidNodeMap[objectUID] = pNode;
								}
							}
							break;

						case ::NM::ODB::ObjectType::Vertex:
							{
								// get vertex group uid
								::NM::ODB::OBJECTUID objectGroupParentUID = _odb->GetValue(objectUID, L"group")->Get<::NM::ODB::ODBUID>();
								// find the vertex group
								GROUPUID_NODEUID_MAP::iterator it =	_groupUidNodeUidMap.find(objectGroupParentUID);
								// if we found the group
								if (it != _groupUidNodeUidMap.end())
								{
									// add the vertex uid to the vector mapped to the group
									it->second.push_back(objectUID);
								}
							}
							break;

						default:
							break;

						}
					}
					break;

				case ::NM::ODB::DatabaseUpdateType::Delete:
					//OutputDebugString(L"\n****DatabaseUpdateType::Delete");
					{						
						switch (record->GetObjectType())
						{
						case ::NM::ODB::ObjectType::Group:
						{
							// delete the item as we have validated its ok if we recieved this.
							// delete group uid to pnode instance
							{							
								GROUPUID_NODE_MAP::iterator it = _groupUidNodeMap.find(record->GetObjectUID());
								if (it != _groupUidNodeMap.end())
								{
									::std::vector<PNODE>::iterator nodeit = ::std::find(_nodes.begin(), _nodes.end(), it->second);
									if (nodeit != _nodes.end())
									{
										delete *nodeit;
										*nodeit = nullptr;
										_nodes.erase(nodeit);
									}
									_groupUidNodeMap.erase(it);
									//OutputDebugString(L"\n\tDeleted Node in Group::PNODE Mapping");
								}								
							}
							// delete the group to vertex vector map
							{
								GROUPUID_NODEUID_MAP::iterator 	it = _groupUidNodeUidMap.find(record->GetObjectUID());
								if (it != _groupUidNodeUidMap.end())
								{
									assert(it->second.size() == 0);
									_groupUidNodeUidMap.erase(it);
									//OutputDebugString(L"\n\tDeleted group in Group::Vertex Mapping");
								}
								//else
								//{
								//	OutputDebugString(L"\n\tNo group::vec<vertex> exists, no children ever assigned");
								//}
							}
						}
						break;

						case ::NM::ODB::ObjectType::Vertex:
						{
							// find the group this vertex was part of
							::NM::ODB::OBJECTUID groupUID = _odb->GetValue(record->GetObjectUID(), L"group")->Get<::NM::ODB::ODBUID>();
							GROUPUID_NODEUID_MAP::iterator 	it = _groupUidNodeUidMap.find(groupUID);
							if (it != _groupUidNodeUidMap.end())
							{
								// find the objectuid in the vector of vertex uids
								::std::vector<::NM::ODB::OBJECTUID>::iterator vecit = ::std::find(it->second.begin(), it->second.end(), record->GetObjectUID());
								if (vecit != it->second.end())
								{
									it->second.erase(vecit);
									OutputDebugString(L"\n\tDeleted Node in Group Mapping");
								}
							}
						}
						break;

						default:
							break;
						}
					}
					break;

				case ::NM::ODB::DatabaseUpdateType::Update:
					OutputDebugString(L"\n****DatabaseUpdateType::Update");
					OutputDebugString(L"\tGroup Table Size:\t");
					OutputDebugString(::std::to_wstring(_odb->GetTableObjectCount(::std::wstring(L"grouptable"))).c_str());
					break;

				case ::NM::ODB::DatabaseUpdateType::Refresh:
					RefreshGroupCache();
					break;

				default:
					break;
				}

			}

			return;
		}


	}
}