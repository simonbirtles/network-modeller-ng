#include "stdafx.h"
#include "Group.h"

/**
*
*
*
*/
Group::Group(::NM::ODB::OBJECTUID groupUID, ::NM::ODB::OBJECTUID parentUID) :
	_groupUID(groupUID),
	_parentUID(parentUID),
	_groupName(L""),
	_scrollX(0),
	_scrollY(0),
	_scaleX(1.0f),
	_scaleY(1.0f)
{
}
Group::Group(::NM::ODB::OBJECTUID groupUID, ::NM::ODB::OBJECTUID parentUID, ::std::wstring groupName) :
	_groupUID(groupUID),
	_parentUID(parentUID),
	_groupName(groupName),
	_scrollX(0),
	_scrollY(0),
	_scaleX(1.0f),
	_scaleY(1.0f)
{

}

/**
*
*
*
*/
Group::~Group()
{
}
/**
*
*
*
*/
::NM::ODB::OBJECTUID Group::GetGroupUID()
{
	return _groupUID;
}
/**
*
*
*
*/
void Group::SetGroupName(CString groupName)
{
	_groupName = groupName;
}
/**
*
*
*
*/
void Group::SetTranslation(int x, int y)
{
	_scrollX = x;
	_scrollY = y;
}
/**
*
*
*
*/
void Group::SetScale(float scaleX, float scaleY)
{
	_scaleX = scaleX;
	_scaleY = scaleY;
}
/**
*
*
*
*/
::std::wstring Group::GetGroupName()
{
	return _groupName;
}
/**
*
*
*
*/
void Group::GetTranslation(int& x, int& y)
{
	x = _scrollX;
	y = _scrollY;
}
/**
*
*
*
*/
void Group::GetScale(float& scaleX, float& scaleY)
{
	scaleX = _scaleX;
	scaleY = _scaleY;
}
/*
*
*
*/
::NM::ODB::OBJECTUID Group::GetParentGroup()
{
	return _parentUID;
}
Group::UIDSET Group::GetChildren()
{
	return _childUIDs;
}
/*
*
*
*/
BOOL Group::IsGlobalGroup()
{
	if (_parentUID == ::NM::ODB::INVALID_OBJECT_UID)
		return TRUE;
	return FALSE;
}
/*
*
*
*/
void Group::SetParentGroup(::NM::ODB::OBJECTUID objectUID)
{
	_parentUID = objectUID;
}
/*
*
*
*/
void Group::AddChildGroup(::NM::ODB::OBJECTUID objectUID)
{
	ASSERT(objectUID != ::NM::ODB::INVALID_OBJECT_UID);
	_childUIDs.insert(objectUID);
}
/*
*
*
*/
void Group::RemoveChildGroup(::NM::ODB::OBJECTUID objectUID)
{
	_childUIDs.erase(objectUID);
}
