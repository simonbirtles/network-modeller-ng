#pragma once
#include <map>
#include <string>
#include "SerialClient.h"


namespace NM 
{
	namespace Serial
	{

		class SerialContext
		{
		public:
			SerialContext();
			~SerialContext();

			typedef ::std::wstring wstr;

			void RegisterClientSerializationService(SerialClient*, ::std::wstring clientRoot);
			void UnregisterClientSerializationService(SerialClient*);

			void SetClientDataDirtyState(SerialClient*);
			bool GetClientDataDirtyState(SerialClient*);
			bool GetGlobalDataDirtyState();

			bool SerializeStateJson();
			bool LoadState(::std::wstring);
			bool SaveState(::std::wstring fileName);

			void NewDatabase();
			wstr GetDatabaseName();


		private:
			// holds the class instance pointer of the client requiring serialization services and if the data needs saving currently.
			typedef bool bDirtyFlag;
			typedef ::std::map<SerialClient*, bDirtyFlag> clientVector;
			clientVector serialClients;

			typedef	::std::wstring rootString;
			typedef ::std::map<rootString, SerialClient*> CLIENTROOTSTRING;
			CLIENTROOTSTRING clientRootString;


			::std::wstring	strDatabaseName;
			bool			bNewGraph;	
		};


//ns
	}
}

