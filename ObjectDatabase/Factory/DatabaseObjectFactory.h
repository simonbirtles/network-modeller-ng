#pragma once

namespace NM 
{
	namespace ODB 
	{
		class Table;
		namespace DefaultTables
		{
			bool	CreateVertexTableSchema(Table* table);
			bool	CreateInterfaceTableSchema(Table* table);
			bool	CreateEdgeTableSchema(Table* table);
			bool	CreateFlowTableSchema(Table* table);
			bool	CreateDemandTableSchema(Table* table);
			bool	CreatePathTableSchema(Table* table);
			bool	CreateGroupTableSchema(Table* table);
			bool	CreateLayerTableSchema(Table* table);
		}
//ns
	}
}