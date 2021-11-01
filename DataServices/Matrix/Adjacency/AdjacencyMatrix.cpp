#include "stdafx.h"
#include "AdjacencyMatrix.h"
#include "..\..\ServiceRegistry\IServiceRegistry.h"
#include "IObjectDatabase.h"
#include <algorithm>
#include <vector>
#include <string>



extern NM::Registry::IServiceRegistry* reg;

namespace NM 
{
	namespace DataServices
	{
		using ::NM::ODB::OBJECTUID;
		/**
		*
		*
		*
		*/
		CAdjacencyMatrix::CAdjacencyMatrix(void)
		{
			::std::unique_ptr<::NM::ODB::IDatabaseUpdate> _updateCache( static_cast<::NM::ODB::IDatabaseUpdate*>(reg->GetClientInterface(L"OBJECTUPDATECACHE")) );
			
			::std::vector<::std::wstring> attributeList;
			attributeList.push_back(L"create");
			attributeList.push_back(L"delete");
			_pUpdateQueue = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Vertex, attributeList) ;
			_pUpdateQueue = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Edge, attributeList);
			attributeList.clear();
			attributeList.push_back(L"*");
			_pUpdateQueue = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Database, attributeList);  //DatabaseUpdateType::Refresh

			_odb.reset( static_cast<::NM::ODB::IObjectDatabase*>(reg->GetClientInterface(L"ODB")) );

			//RebuildMatrix();
		}
		/**
		*
		*
		*
		*/
		CAdjacencyMatrix::~CAdjacencyMatrix(void)
		{
			::std::unique_ptr<::NM::ODB::IDatabaseUpdate> _updateCache(static_cast<::NM::ODB::IDatabaseUpdate*>(reg->GetClientInterface(L"OBJECTUPDATECACHE")));
			_updateCache->DeleteClientUpdatesQueue(this);

		}

		/**
		*
		*
		*
		*/
		void CAdjacencyMatrix::RebuildMatrix()
		{
			Clear();
			assert(_odb);

			// loop through and add all verticies (nodes).
			::std::wstring table = L"vertextable";
			OBJECTUID objectUID = _odb->GetFirstObject(table);

			while (objectUID != ::NM::ODB::INVALID_OBJECT_UID)
			{
				CreateVertex(objectUID);
				// next object
				objectUID = _odb->GetNextObject(objectUID);
			}

			
			// loop through and add all edges (links).
			table = L"edgetable";
			objectUID = _odb->GetFirstObject(table);
			while (objectUID != ::NM::ODB::INVALID_OBJECT_UID)
			{
				// get connected vertex A
				::NM::ODB::OBJECTUID intfA = _odb->GetValue(objectUID, L"interfaceUID_A")->Get<::NM::ODB::OBJECTUID>();
				assert(intfA != ::NM::ODB::INVALID_OBJECT_UID);
				::NM::ODB::OBJECTUID vertexA = _odb->GetValue(intfA, L"vertexUID")->Get<::NM::ODB::OBJECTUID>();
				assert(vertexA != ::NM::ODB::INVALID_OBJECT_UID);
				// get connected vertex b
				::NM::ODB::OBJECTUID intfB = _odb->GetValue(objectUID, L"interfaceUID_B")->Get<::NM::ODB::OBJECTUID>();
				assert(intfB != ::NM::ODB::INVALID_OBJECT_UID);
				::NM::ODB::OBJECTUID vertexB = _odb->GetValue(intfB, L"vertexUID")->Get<::NM::ODB::OBJECTUID>();
				assert(vertexB != ::NM::ODB::INVALID_OBJECT_UID);
				// add edge
				if ((vertexA != ::NM::ODB::INVALID_OBJECT_UID) && (vertexA != ::NM::ODB::INVALID_OBJECT_UID))
					CreateEdge(vertexA, vertexB, objectUID, EDGETYPE::EDGE_UNDIRECTED);


				// next link
				objectUID = _odb->GetNextObject(objectUID);
			}
			
			return;
		}
		/**
		*
		*
		*
		*/
		void CAdjacencyMatrix::DatabaseUpdate()
		{
			while (!_pUpdateQueue->Empty())
			{
				::std::shared_ptr<::NM::ODB::DatabaseUpdateRecord> record = _pUpdateQueue->GetNextRecord();
				::NM::ODB::DatabaseUpdateType updateType = record->GetUpdateType();
				::NM::ODB::OBJECTUID objectUID = record->GetObjectUID();
				switch (updateType)
				{

				case ::NM::ODB::DatabaseUpdateType::Create:
				{

					switch (record->GetObjectType())
					{											

					case ::NM::ODB::ObjectType::Vertex:
					{
						CreateVertex(objectUID);
					}
					break;

					case ::NM::ODB::ObjectType::Edge:
					{
						// get connected vertex A
						::NM::ODB::OBJECTUID intfA = _odb->GetValue(objectUID, L"interfaceUID_A")->Get<::NM::ODB::OBJECTUID>();
						assert(intfA != ::NM::ODB::INVALID_OBJECT_UID);
						::NM::ODB::OBJECTUID vertexA = _odb->GetValue(intfA, L"vertexUID")->Get<::NM::ODB::OBJECTUID>();
						assert(vertexA != ::NM::ODB::INVALID_OBJECT_UID);
						// get connected vertex b
						::NM::ODB::OBJECTUID intfB = _odb->GetValue(objectUID, L"interfaceUID_B")->Get<::NM::ODB::OBJECTUID>();
						assert(intfB != ::NM::ODB::INVALID_OBJECT_UID);
						::NM::ODB::OBJECTUID vertexB = _odb->GetValue(intfB, L"vertexUID")->Get<::NM::ODB::OBJECTUID>();
						assert(vertexB != ::NM::ODB::INVALID_OBJECT_UID);
						// add edge
						if((vertexA != ::NM::ODB::INVALID_OBJECT_UID) && (vertexA != ::NM::ODB::INVALID_OBJECT_UID))
							CreateEdge(vertexA, vertexB, objectUID, EDGETYPE::EDGE_UNDIRECTED);

					}
					break;

					default:
						break;
					}
				}
				break;  // case ::NM::ODB::DatabaseUpdateType::Create:



				case ::NM::ODB::DatabaseUpdateType::Delete:
				{
					switch (record->GetObjectType())
					{

					case ::NM::ODB::ObjectType::Vertex:
					{
						DeleteVertex(objectUID);
					}
					break;

					case ::NM::ODB::ObjectType::Edge:
					{
						// get connected vertex A
						::NM::ODB::OBJECTUID intfA = _odb->GetValue(objectUID, L"interfaceUID_A")->Get<::NM::ODB::OBJECTUID>();
						assert(intfA != ::NM::ODB::INVALID_OBJECT_UID);
						::NM::ODB::OBJECTUID vertexA = _odb->GetValue(intfA, L"vertexUID")->Get<::NM::ODB::OBJECTUID>();
						assert(vertexA != ::NM::ODB::INVALID_OBJECT_UID);
						// get connected vertex b
						::NM::ODB::OBJECTUID intfB = _odb->GetValue(objectUID, L"interfaceUID_B")->Get<::NM::ODB::OBJECTUID>();
						assert(intfB != ::NM::ODB::INVALID_OBJECT_UID);
						::NM::ODB::OBJECTUID vertexB = _odb->GetValue(intfB, L"vertexUID")->Get<::NM::ODB::OBJECTUID>();
						assert(vertexB != ::NM::ODB::INVALID_OBJECT_UID);
						// add edge
						if ((vertexA != ::NM::ODB::INVALID_OBJECT_UID) && (vertexA != ::NM::ODB::INVALID_OBJECT_UID))
							DeleteEdge(vertexA, vertexB, objectUID, EDGETYPE::EDGE_UNDIRECTED);
					}
					break;

					default:
						break;
					}
				}
				break;  // case ::NM::ODB::DatabaseUpdateType::Delete:



				case ::NM::ODB::DatabaseUpdateType::Update:
				{
					assert(false);
					switch (record->GetObjectType())
					{

					case ::NM::ODB::ObjectType::Vertex:
					{
					}
					break;

					case ::NM::ODB::ObjectType::Edge:
					{
					}
					break;

					default:
						break;
					}
				}
				break;  // case ::NM::ODB::DatabaseUpdateType::Update:



				case ::NM::ODB::DatabaseUpdateType::Refresh:
				{
					//Clear & Build Matrix();
					RebuildMatrix();					
				}
				break;	// case ::NM::ODB::DatabaseUpdateType::Refresh:




				default:
					break;
				}

			}
		}


		// ns
	}
}