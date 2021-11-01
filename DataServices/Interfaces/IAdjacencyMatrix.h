#pragma once
#include "..\..\ObjectDatabase\Interfaces\ObjectDatabaseDefines.h"
#include <memory>
/*
	Public readonly wrapper for database AdjacencyMatrix
*/

namespace NM 
{
	namespace DataServices 
	{
		class CAdjacencyMatrix;

		class IAdjacencyMatrix
		{
		public:
			explicit IAdjacencyMatrix(::std::shared_ptr<CAdjacencyMatrix>);
			~IAdjacencyMatrix(void);
			
			size_t	GetVertexConnectedVerticies(::NM::ODB::OBJECTUID VertexID,::std::vector<::NM::ODB::OBJECTUID> &vertexlist);
			size_t	GetVertexConnectedEdgeCount(::NM::ODB::OBJECTUID srcVertexID, ::NM::ODB::OBJECTUID dstVertexID);
			void	GetVertexConnectedEdges(::NM::ODB::OBJECTUID srcVertexID, ::NM::ODB::OBJECTUID dstVertexID, ::std::vector<::NM::ODB::OBJECTUID> &edgelist);
			void	GetVertexEdges(::NM::ODB::OBJECTUID VertexID, ::std::vector<::NM::ODB::OBJECTUID> &edgelist);
			size_t	GetVertexDegree(::NM::ODB::OBJECTUID VertexID);
			void	GetMatrixSize(size_t &rows, size_t &cols);

		private:
			IAdjacencyMatrix();
			::std::shared_ptr<CAdjacencyMatrix>	_pMatrix;

		};
		// ns
	}
}

