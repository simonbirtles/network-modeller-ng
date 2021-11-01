#pragma once
#include "Interfaces\ObjectDatabaseDefines.h"
#include "Interfaces\IDatabaseUpdate.h"					// for client interface 
#include "Interfaces\DatabaseObserver.h"				// base class for client listener (dervive from for update calls)
#include "Group.h"
#include "helpers.h"

class Groups : public::NM::ODB::CDatabaseObserver
{

public:
	Groups();
	~Groups();

	::NM::ODB::OBJECTUID GetActiveGroup();

	BOOL SetActiveGroup(::NM::ODB::OBJECTUID groupUID);
	void SetTranslation(int x, int y);
	void SetScale(float scaleX, float scaleY);

	::std::wstring GetGroupName(::NM::ODB::OBJECTUID groupUID);
	void GetTranslation(int& x, int& y);
	void GetScale(float& scaleX, float& scaleY);

	// from DatabaseUpdateCache
	void DatabaseUpdate();

private:
	::NM::ODB::OBJECTUID _currentGroupUID;

	typedef ::std::shared_ptr<Group> SPGROUP;
	typedef Group GROUP;
	typedef ::std::map<::NM::ODB::OBJECTUID, SPGROUP, ::NM::HELPERS::compareGuid> UID_GROUP_MAP;
	UID_GROUP_MAP _groups;
	::std::unique_ptr<::NM::ODB::IObjectDatabase>				_odb;					// interface to the database
	::std::unique_ptr<NM::ODB::IDatabaseUpdate>					_updateCache;			// interface to the db update cache
	::NM::ODB::IDatabaseUpdate::UpdateQueueHandle				_updateQ;				// out queue of updates to db we requested


	SPGROUP GetGroup(::NM::ODB::OBJECTUID);
	void RefreshGroups();
	void AddGroup(::NM::ODB::OBJECTUID objectUID, ::NM::ODB::OBJECTUID parentUID,  std::wstring groupName);
	BOOL SetGroupName(::NM::ODB::OBJECTUID groupUID, CString groupName);
	BOOL ValidateGroupTree();
	BOOL DeleteGroup(::NM::ODB::OBJECTUID objectUID);
};

