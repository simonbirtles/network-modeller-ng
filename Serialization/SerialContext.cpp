#include "stdafx.h"
#include "SerialContext.h"
#include "..\TinyXML-2\tinyxml2.h"

extern void WCtMB(const ::std::wstring &ws, ::std::string &mbs);
extern void MBtWC(::std::wstring &ws, const ::std::string &mbs);

namespace NM 
{
	namespace Serial
	{

		/**
		*
		*
		**/
		SerialContext::SerialContext():
			strDatabaseName(L"")
		{
		}
		/**
		*
		*
		**/
		SerialContext::~SerialContext()
		{
			serialClients.clear();
			clientRootString.clear();
		}
		/**
		*
		*
		**/
		void SerialContext::RegisterClientSerializationService(SerialClient *pClient, ::std::wstring clientRoot)
		{
			if(pClient == nullptr) return;

			clientVector::iterator it = serialClients.begin();
			// does client already exist in the db ?
			it = serialClients.find(pClient);
			if(it != serialClients.end())
			{
				// TODO need to log and return a failure code as here we already have the client registered
				return;
			}

			CLIENTROOTSTRING::iterator itRoot = clientRootString.find(clientRoot);
			if(itRoot != clientRootString.end())
			{
				// TODO need to log and return a failure code as here we already have a client registered with same root name
				return;
			}
			
			// client does not exist, so add it and declare data not dirty flag
			serialClients[pClient] = false;
			// add rootName -> client pointer map
			clientRootString[clientRoot] = pClient;	
	
			return;
		}
		/**
		*
		*
		**/
		void SerialContext::UnregisterClientSerializationService(SerialClient* pClient)
		{
			if(pClient == nullptr) return;

			clientVector::iterator it = serialClients.begin();
			// does client already exist in the db ?
			it = serialClients.find(pClient);
			if(it != serialClients.end())
			{
				// client does exist, so delete it (unregister)
				serialClients.erase(it);
			}	

			// find and delete corrosponding rootName --> client entry
			CLIENTROOTSTRING::iterator itRoot = clientRootString.begin();
			while( itRoot != clientRootString.end() )
			{
				if( itRoot->second == pClient )
				{
					clientRootString.erase(itRoot);
					break;
				}
				++itRoot;
			}
			return;
		}
		/**
		*
		*
		**/
		void SerialContext::SetClientDataDirtyState(SerialClient* pClient)
		{
			if(pClient == nullptr) return;

			clientVector::iterator it = serialClients.begin();
			// does client exist in the db ?
			it = serialClients.find(pClient);
			if(it != serialClients.end())
			{
				// client does exist, so update data dirty flag
				serialClients[pClient] = true;
			}	
		}
		/**
		*
		*
		**/
		bool SerialContext::GetClientDataDirtyState(SerialClient* pClient)
		{
			if(pClient == nullptr) return false;

			clientVector::iterator it = serialClients.begin();
			// does client exist in the db ?
			it = serialClients.find(pClient);
			if(it != serialClients.end())
			{
				return it->second;		// return the boolean dirty flag
			}	

			return false;
		}
		/**
		*
		* check to see if any clients have dirty data, 
		* if we find just one then return true data dirty flag otherwise false
		**/
		bool SerialContext::GetGlobalDataDirtyState()
		{	
			clientVector::iterator it = serialClients.begin();
			while(it != serialClients.end())
			{
				if(it->second)
					return true;
				++it;
			}
			return false;
		}
		/*
		*	
		* LoadState
		*
		* 
		*/
		bool SerialContext::LoadState(::std::wstring filename)
		{
			/*
			tinyXML2 to open document
			check root
			for each child of root, call relevent client to load data up (client function - LoadXMLData(...) )

			- maybe do something to pause all services like SPF during load to prevent problems ? would need them to register with service control and halt ?? hmmm

			*/
			::std::string mbStrFilename; 
			WCtMB(filename, mbStrFilename);
			tinyxml2::XMLDocument doc;
			tinyxml2::XMLError error = doc.LoadFile(mbStrFilename.c_str());

			if( error != tinyxml2::XML_SUCCESS )
			{
				//log and quit
				return false;
			}

			// check root
			if( ::std::string(doc.FirstChild()->Value()).compare("root") != 0)
			{
				// todo LOG 
				return false;
			}

			// loop through children of root and call client if one registered for each 1st level child.
			tinyxml2::XMLNode *currentNode = doc.FirstChild()->FirstChild(); // should be at <root> --> <????> which should match a registered client string

			for( tinyxml2::XMLNode *node=currentNode; node; node = node->NextSibling() )
			{
				// get the node string (value)
				::std::wstring rootName;
				MBtWC(rootName, node->Value());

				// check to see if it matches a registered client
				CLIENTROOTSTRING::iterator itRoot = clientRootString.find(rootName);
				if( itRoot != clientRootString.end() )
				{
					// found a match, then call clients LoadXMLData function - do we need to pass the document pointer ?? send a nullptr to check but seems not !
					tinyxml2::XMLNode *clientNode = node;		// dont pass our copy of the pointer as it would be changed by the clients !
					bool success = itRoot->second->LoadXMLData(nullptr, node);
					// todo LOG success for rootName.
				}
				else
				{
					// TODO Log rootName as unowned in this app instance
					OutputDebugString(L"\nSerialContext::LoadState Unknown root client name in XML (no client registered with this node root name)");
					OutputDebugString(L"\ncontinuing to load file for other registered clients.");
				}
				// next client string root...
			}

			// todo LOG complete
			return true;
		}
		/**
		*
		*
		**/
		bool SerialContext::SaveState(::std::wstring fileName)
		{
			tinyxml2::XMLError error;

			// create the XMLDocument
			tinyxml2::XMLDocument doc;

			// Add The Root
			tinyxml2::XMLElement *root = doc.NewElement("root");
			doc.InsertFirstChild(root);

			// Call each registered client whos data is flagged dirty
			clientVector::iterator itClient = serialClients.begin();
			while( itClient != serialClients.end() )
			{
				// TODO: Require dirtyflag check itClient->second is boolean dirty flag
				itClient->first->SaveXMLData(&doc, root); 
				++itClient;
			}

			// Save the file
			::std::string mbFilename;
			WCtMB(fileName, mbFilename);
			error = doc.SaveFile(mbFilename.c_str());
			// check error

			return true;
		}
		/**
		* Request to serialize (save) data. 
		*
		* We contact each client registered for serialization services and "ask" for the data to be 
		* saved in a JSON format with the outer object labelled as the client wishes but is aware of 
		* to request the same "named" data when a archive is opened. 
		**/
		bool SerialContext::SerializeStateJson()
		{
			//// for each client to post data
			//Serial::JsonSerialize clientJson;
			//// for all client data aggregated
			//Serial::JsonSerialize appJson;

			//clientVector::iterator it = SerialClients.begin();
			//while(it != SerialClients.end())
			//{
			//	// ask first client for data (returns true if data is provided else we ignore this one)
			//	if( it->first->SerialiseData(clientJson) )
			//	{
			//		// TODO : Need a JSON validator here or one clients bad JSON will mess up the opening of the file.
			//		// add the Json to the final appJson object with other db data
			//		appJson.InsertJsonObject(&clientJson);
			//	}

			//	// clear for next client
			//	clientJson.ClearJson();
			//	// next client
			//	++it;
			//}

			//// Get the path and filename we are to save as.
			//::std::wstring temp = strDatabaseName;
			//if(bNewGraph)
			//{
			//	temp = GetDatabasePathName();
			//}

			//if( WriteToFile(appJson, strDatabaseName) )
			//{
			//	strDatabaseName = temp;

			//	// clear dirty flags
			//	clientVector::iterator it = SerialClients.begin();
			//	while( it != SerialClients.end() )
			//	{
			//		// set dirty flag to false as we have saved database
			//		it->second = false;
			//		++it;
			//	}

			//	return true;
			//}
			//
			return false;
		}
		/**
		*
		*
		**/
		void SerialContext::NewDatabase()
		{
			strDatabaseName = L"";
			bNewGraph = true;

			clientVector::iterator it = serialClients.begin();
			while( it != serialClients.end() )
			{
				// set dirty flag to false as this is a new database.
				it->second = false;
				++it;
			}
		}
		/**
		*
		*
		**/
		::std::wstring SerialContext::GetDatabaseName()
		{
			return strDatabaseName;
		}

 // namespace
	}
}; 