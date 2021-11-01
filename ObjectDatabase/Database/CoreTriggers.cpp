#include "stdafx.h"
#include "CoreTriggers.h"
#include "triggermanager.h"
#include "..\interfaces\iobjectdatabase.h"
#include "IAdjacencyMatrix.h"
#include "..\..\ServiceRegistry\IServiceRegistry.h"							// registry interface

#include <assert.h>

extern NM::Registry::IServiceRegistry* reg;

namespace  NM
{
	namespace ODB
	{
		/**
		*
		*
		*/
		CoreTriggers::CoreTriggers()
		{
		}
		/**
		*
		*
		*/
		CoreTriggers::~CoreTriggers()
		{
		}
		/**
		*
		*
		*/
		void CoreTriggers::CreateCoreTriggers(::std::shared_ptr<TriggerManager> triggerManager)
		{
			CreateCoreTrigger_f1(triggerManager);
			CreateCoreTrigger_f2(triggerManager);
			CreateCoreTrigger_f3(triggerManager);
			CreateCoreTrigger_f4(triggerManager);
		}
		/*
		* CreateCoreTrigger_f1
		* Operation: Update
		* TableName: edgetable
		* AttributeName: interfaceUID_A & interfaceUID_B
		* 
		* On change of an edges connected verticies
		* ensure edge connected verticies are actually verticies and are not the same vertex
		* if either connected vertex is inservice:false, make edge inservice:false
		* 
		*/
		void CoreTriggers::CreateCoreTrigger_f1(::std::shared_ptr<TriggerManager> triggerManager)
		{
			///>>
			TriggerFunc f1 = [](IObjectDatabase& odb, OBJECTUID uid, const Value& pValue, const Value& nValue)
			{
				// get connected vertex A
				::NM::ODB::OBJECTUID intfA = odb.GetValue(uid, L"interfaceUID_A")->Get<::NM::ODB::OBJECTUID>();
				assert(intfA != ::NM::ODB::INVALID_OBJECT_UID);
				::NM::ODB::OBJECTUID vertexA = odb.GetValue(intfA, L"vertexUID")->Get<::NM::ODB::OBJECTUID>();
				assert(vertexA != ::NM::ODB::INVALID_OBJECT_UID);
				// get connected vertex b
				::NM::ODB::OBJECTUID intfB = odb.GetValue(uid, L"interfaceUID_B")->Get<::NM::ODB::OBJECTUID>();
				assert(intfB != ::NM::ODB::INVALID_OBJECT_UID);
				::NM::ODB::OBJECTUID vertexB = odb.GetValue(intfB, L"vertexUID")->Get<::NM::ODB::OBJECTUID>();
				assert(vertexB != ::NM::ODB::INVALID_OBJECT_UID);

				// either may not be set yet so allow it to go ahead, when both set we can check again.
				if ((vertexA == INVALID_OBJECT_UID) ||
					(vertexB == INVALID_OBJECT_UID))
				{
					return true;
				}

				// check they are not the same endpoints
				if (vertexA == vertexB)
				{
					// todo log
					return false;
				}

				// Validate both given connected verticies are actually routers (or interfaces when we use interfaces) and are different 
				if ((odb.GetObjectTypeName(vertexA) != L"vertex") ||
					(odb.GetObjectTypeName(vertexB) != L"vertex"))
				{
					// todo log
					return false;
				}

				// Validate both verticies are inservice, else make this out of service.
				ODBBool vertexAinservice = odb.GetValue(vertexA, L"inservice")->Get<ODBBool>();
				ODBBool vertexBinservice = odb.GetValue(vertexB, L"inservice")->Get<ODBBool>();
				if (!vertexAinservice || !vertexBinservice)
				{
					odb.SetValue(uid, L"inservice", real_bool(false));
				}

				return true;
			};
			///>>

			::std::wstring tableName = L"edgetable";
			::std::wstring attributeName = L"interfaceUID_A";
			triggerManager->CreateTrigger(tableName, attributeName, TriggerOperation::Update, f1);
			attributeName = L"interfaceUID_B";
			triggerManager->CreateTrigger(tableName, attributeName, TriggerOperation::Update, f1);
			return;
		}
		/*
		* CreateCoreTrigger_f2
		* Operation: Update
		* TableName: vertextable
		* AttributeName: inservice
		*
		* if a vertex is set out of service then set connected edges out of service too
		*/
		void CoreTriggers::CreateCoreTrigger_f2(::std::shared_ptr<TriggerManager> triggerManager)
		{
			
			///>>
			TriggerFunc f2 = [](IObjectDatabase& odb, OBJECTUID uid, const Value& pValue, const Value& nValue)
			{
				// are we going inservice or out of service ?
				if (nValue.Get<ODBBool>() == true)
				{
					return true;	// going in service so nothing for us to check.
				}

				// if the global registry is not available we are in trouble
				if (!reg)
					throw ::std::runtime_error("Application Registry Not Available, Cannot Continue.");

				::std::unique_ptr<::NM::DataServices::IAdjacencyMatrix>		_adjm;

				_adjm.reset(static_cast<::NM::DataServices::IAdjacencyMatrix*>(reg->GetClientInterface(L"ADJMATRIX")));
				if (!_adjm)
					throw ::std::runtime_error("Adj. Matrix Service Not Available, Cannot Continue.");

				::std::vector<OBJECTUID> connectedEdges;
				// get the edges connected to this vertex
				_adjm->GetVertexEdges(uid, connectedEdges);
				// set all attached edges out of service
				for (size_t t = 0; t < connectedEdges.size(); ++t)
				{
					odb.SetValue(connectedEdges[t], L"inservice", real_bool(false));
				}
				return true;
			};
			///>>

			::std::wstring tableName = L"vertextable";
			::std::wstring attributeName = L"inservice";
			triggerManager->CreateTrigger(tableName, attributeName, TriggerOperation::Update, f2);
			return;
		}
		/*
		* CreateCoreTrigger_f3
		* Operation: Delete
		* TableName: vertextable
		* AttributeName: *
		*
		* if a vertex is deleted then delete connected interfaces & edges too
		*/
		void CoreTriggers::CreateCoreTrigger_f3(::std::shared_ptr<TriggerManager> triggerManager)
		{		
			///>>
			TriggerFunc f3 = [](IObjectDatabase& odb, OBJECTUID uid, const Value& pValue, const Value& nValue)
			{
				// if the global registry is not available we are in trouble
				if (!reg) throw ::std::runtime_error("Application Registry Not Available, Cannot Continue.");

				::std::unique_ptr<::NM::DataServices::IAdjacencyMatrix>		_adjm;
				_adjm.reset(static_cast<::NM::DataServices::IAdjacencyMatrix*>(reg->GetClientInterface(L"ADJMATRIX")));
				if (!_adjm)	throw ::std::runtime_error("Adj. Matrix Service Not Available, Cannot Continue.");
				
				// get the edges connected to this vertex and  delete all attached edges
				::std::vector<OBJECTUID> connectedEdges;
				_adjm->GetVertexEdges(uid, connectedEdges);
				for (size_t t = 0; t < connectedEdges.size(); ++t)
					odb.DeleteObject(connectedEdges[t]);
				
				// delete all interfaces connected to this vertex
				UPVALUE pVecIntf = odb.GetValue(uid, L"interfaces");
				if (pVecIntf)
				{
					::NM::ODB::ODBVectorUID interfaces = pVecIntf->Get<::NM::ODB::ODBVectorUID>();
					for each(::NM::ODB::OBJECTUID intfUID in interfaces)
						odb.DeleteObject(intfUID);
				}

				return true;
			};
			///>>

			::std::wstring tableName = L"vertextable";
			::std::wstring attributeName = L"";	// not required for a delete operation
			triggerManager->CreateTrigger(tableName, attributeName, TriggerOperation::Delete, f3);
			return;
		}
		/*
		* CreateCoreTrigger_f4
		* Operation: Update
		* TableName: edgetable
		* AttributeName: inservice
		*
		* if an edge is set inservice:true and one or both of the connected verticies are not, then disallow edge inservice:true
		*
		*/
		void CoreTriggers::CreateCoreTrigger_f4(::std::shared_ptr<TriggerManager> triggerManager)		
		{			
			///>>
			TriggerFunc f4 = [](IObjectDatabase& odb, OBJECTUID uid, const Value& pValue, const Value& nValue)
			{
				// are we going inservice or outof service ?
				if (nValue.Get<ODBBool>() == false)
				{
					return true;	// going out of service so nothing for us to check.
				}

				// get UIDs of connected verticies
				// get connected vertex A
				::NM::ODB::OBJECTUID intfA = odb.GetValue(uid, L"interfaceUID_A")->Get<::NM::ODB::OBJECTUID>();
				assert(intfA != ::NM::ODB::INVALID_OBJECT_UID);
				::NM::ODB::OBJECTUID vertexA = odb.GetValue(intfA, L"vertexUID")->Get<::NM::ODB::OBJECTUID>();
				assert(vertexA != ::NM::ODB::INVALID_OBJECT_UID);
				// get connected vertex b
				::NM::ODB::OBJECTUID intfB = odb.GetValue(uid, L"interfaceUID_B")->Get<::NM::ODB::OBJECTUID>();
				assert(intfB != ::NM::ODB::INVALID_OBJECT_UID);
				::NM::ODB::OBJECTUID vertexB = odb.GetValue(intfB, L"vertexUID")->Get<::NM::ODB::OBJECTUID>();
				assert(vertexB != ::NM::ODB::INVALID_OBJECT_UID);

				if ((vertexA == INVALID_OBJECT_UID) || (vertexB == INVALID_OBJECT_UID))
				{
					// this situation may occur if the attribute inservice is set before the connected verticies
					// therefore we return true to avoid stopping the creation and further validate when the 
					// connected verticies are updated, other code should check for INVALID_OBJECT_UID anyway
					// so will detect the invalid and not use the edge anyway. (hopefully...)
					// all calls to DB with INVALID_OBJECT_UID will return false or INVALID_OBJECT_UID
					return true;
				}

				// are both verticies are inservice, else make this out of service.
				ODBBool vertexAinservice = odb.GetValue(vertexA, L"inservice")->Get<ODBBool>();
				ODBBool vertexBinservice = odb.GetValue(vertexB, L"inservice")->Get<ODBBool>();

				// are both verticies are inservice, else dont allow inservice=true for edge
				if (!vertexAinservice || !vertexBinservice)
				{
					return false;
				}
				return true;
			};
			///>>
		
			::std::wstring tableName = L"edgetable";
			::std::wstring attributeName = L"inservice";
			triggerManager->CreateTrigger(tableName, attributeName, TriggerOperation::Update, f4);
			return;
		}

		// ns
	}
}