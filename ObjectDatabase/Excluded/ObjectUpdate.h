#pragma once
#include <map>
#include <string>


namespace NM {
	namespace ODB {

	enum class DatabaseUpdateType;
	class AttributeUpdate;
	class DatabaseUpdateRecord;

	class ObjectUpdate
	{
	public:
		ObjectUpdate(void);											// constructor
		ObjectUpdate(const ObjectUpdate& rhs) ;						// copy constructor
		~ObjectUpdate(void);										// destructor

		void				SetUpdateType(DatabaseUpdateType update_type);
		DatabaseUpdateType	GetUpdateType();
		void				InsertUpdate(DatabaseUpdateRecord &update_record);
		

	private:
		ObjectUpdate &operator=(const ObjectUpdate &rhs);			// assignment 
		

		typedef ::std::map<::std::wstring, AttributeUpdate*> attr_map;
		attr_map _attribute_update_map;

		DatabaseUpdateType	_update_type;


	};

}
}