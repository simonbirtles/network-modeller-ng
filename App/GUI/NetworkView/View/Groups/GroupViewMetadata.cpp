#include "stdafx.h"
#include "GroupViewMetadata.h"

/**
*
*
*
*/
GroupViewMetadata::GroupViewMetadata(::NM::ODB::OBJECTUID groupUID) :
	_groupUID(groupUID),
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
GroupViewMetadata::~GroupViewMetadata()
{
}
/**
*
*
*
*/
::NM::ODB::OBJECTUID GroupViewMetadata::GetGroupUID()
{
	return _groupUID;
}
/**
*
*
*
*/
void GroupViewMetadata::SetTranslation(int x, int y)
{
	_scrollX = x;
	_scrollY = y;
}
/**
*
*
*
*/
void GroupViewMetadata::SetScale(float scaleX, float scaleY)
{
	_scaleX = scaleX;
	_scaleY = scaleY;
}
/**
*
*
*
*/
void GroupViewMetadata::GetTranslation(int &x, int &y)
{
	x = _scrollX;
	y = _scrollY;
}
/**
*
*
*
*/
void GroupViewMetadata::GetScale(float &scaleX, float &scaleY)
{
	scaleX = _scaleX;
	scaleY = _scaleY;
}

