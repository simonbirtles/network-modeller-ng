#pragma once
#include "Interfaces\ObjectDatabaseDefines.h"
#include "helpers.h"
#include <set>

class Group
{
	friend class Groups;

	typedef ::std::set<::NM::ODB::OBJECTUID, ::NM::HELPERS::compareGuid> UIDSET;

public:
	Group(::NM::ODB::OBJECTUID groupUID, ::NM::ODB::OBJECTUID parentUID);
	Group(::NM::ODB::OBJECTUID groupUID, ::NM::ODB::OBJECTUID parentUID, ::std::wstring groupName);
	~Group();

	::NM::ODB::OBJECTUID GetGroupUID();

	void SetGroupName(CString groupName);
	void SetTranslation(int x, int y);
	void SetScale(float scaleX, float scaleY);

	::std::wstring GetGroupName();
	void GetTranslation(int& x, int& y);
	void GetScale(float& scaleX, float& scaleY);
	::NM::ODB::OBJECTUID GetParentGroup();
	UIDSET GetChildren();

	BOOL IsGlobalGroup();


private:
	::NM::ODB::OBJECTUID _groupUID;
	::NM::ODB::OBJECTUID _parentUID;
	UIDSET  _childUIDs;
	::std::wstring _groupName;
	int		_scrollX;
	int		_scrollY;
	float	_scaleX;
	float	_scaleY;

	void SetParentGroup(::NM::ODB::OBJECTUID objectUID);
	void AddChildGroup(::NM::ODB::OBJECTUID objectUID);
	void RemoveChildGroup(::NM::ODB::OBJECTUID objectUID);

};

