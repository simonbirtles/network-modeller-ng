#pragma once
#include <map>
#include "..\Interfaces\ObjectDatabaseDefines.h"


namespace NM 
{
	namespace ODB 
	{
		class DatabaseObject;
		class AttributeOptionList;

		class DatabaseObjectFactory
		{
		public:
			DatabaseObjectFactory();
			~DatabaseObjectFactory();
			
			DatabaseObject*	GetVertexDefaultObject();
			DatabaseObject*	GetInterfaceDefaultObject();
			DatabaseObject*	GetEdgeDefaultObject();
			DatabaseObject*	GetFlowDefaultObject();
			DatabaseObject*	GetDemandDefaultObject();
			DatabaseObject*	GetPathDefaultObject();
			DatabaseObject*	GetGroupDefaultObject();
			DatabaseObject*	GetLayerDefaultObject();			
			//void			AddFlowProtocol(IPProtocol Protocol, DatabaseObject* flow);		

		private:	

			void CreateDefaultOptionLists();
			::std::shared_ptr<AttributeOptionList> attrOptionListTimeUnit;
			::std::shared_ptr<AttributeOptionList> optListBandwidthUnitsList;
			::std::shared_ptr<AttributeOptionList> attrOptionListLineType;
			::std::shared_ptr<AttributeOptionList> attrOptionListVertexType;
			::std::shared_ptr<AttributeOptionList> optListInterfaceTypeList;
			::std::shared_ptr<AttributeOptionList> optListLinkTypeList;



		};
//ns
	}
}