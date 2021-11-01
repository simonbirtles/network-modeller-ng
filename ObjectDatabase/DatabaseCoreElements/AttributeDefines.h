#pragma once


namespace NM
{
	namespace ODB
	{

		

	}
}


#define ATTR_SYSTEM                     0x01			// cannot be deleted (can it be changed ????? ATTR_READ_ONLY ??)
#define ATTR_READ_ONLY                  0x02			// enforced read only 
#define ATTR_USER_MODIFY                0x04			// suggested user does not directly/raw modify
#define ATTR_VALUE_REQUIRED_ON_CREATE   0x08			// when attribute/object is first created a value must be assigned
#define ATTR_SERIALISE                  0x10			// save attribute to disk
#define ATTR_LOCKED_ON_SET              0x20			// once the value is first set it is locked from further changes
#define ATTR_OPTIONS_LIST               0x40			// attibute has a options list which is enforced when setting the value
#define ATTR_USER_CREATE				0x80			// user created this attribute, can be deleted by external code
#define ATTR_ALL						0xFFFF

