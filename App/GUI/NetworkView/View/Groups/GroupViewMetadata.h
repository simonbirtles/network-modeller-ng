#pragma once
#include "Interfaces\ObjectDatabaseDefines.h"

class GroupViewMetadata
{
public:
	GroupViewMetadata(::NM::ODB::OBJECTUID groupUID);
	~GroupViewMetadata();

	::NM::ODB::OBJECTUID GetGroupUID();

	void	SetTranslation(int x, int y);
	void	SetScale(float scaleX, float scaleY);

	void	GetTranslation(int &x, int &y);
	void	GetScale(float &scaleX, float &scaleY);

private:
	::NM::ODB::OBJECTUID _groupUID;
	int		_scrollX;
	int		_scrollY;
	float	_scaleX;
	float	_scaleY;


};

