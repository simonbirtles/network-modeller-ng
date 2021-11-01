#pragma once
#include "..\Database\ObjectDatabase.h"
#include <string>

namespace NM {
	namespace ODB {

class DatabaseObject;

bool UpdateSelectedList(DatabaseObject *go, const Value &value)
{
	bool bselected = value.Get<bool>();
	//CObjectDatabase *db = CObjectDatabase::getInstance();

	if(bselected)
	{
// ***** TODO SELECT ***** ///		db->SelectObject(go);		
	}
	else
	{
		// ***** TODO SELECT ***** ///db->DeselectObject(go);
	}

	
	return true;
}

bool ValidateRGB(DatabaseObject *go,const Value &value)
{
	int RGBValue = value.Get<int>();

	if((RGBValue > 255) || (RGBValue < 0))
	{
		return false;
	}
	
	return true;
}

bool ValidateShortName(DatabaseObject *go,const Value &value)
{

	::std::wstring shortname  = value.Get<::std::wstring>();

	if(shortname.size() > 20)
	{
		return false;
	}

	return true;

}

bool ValidateTransparency(DatabaseObject *go,const Value &value)
{
	int val = value.Get<int>();

	if((val > 100) || (val < 0))
	{
		return false;
	}
	
	return true;


}

bool ValidateWeight(DatabaseObject *go,const Value &value)
{
	int val = value.Get<int>();

	if((val > 65535) || (val < 0))
	{
		return false;
	}

	return true;
}

	}
}