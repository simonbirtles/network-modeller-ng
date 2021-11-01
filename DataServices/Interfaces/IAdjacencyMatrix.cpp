#include "stdafx.h"
#include "IAdjacencyMatrix.h"
#include "AdjacencyMatrix.h"
#include <vector>

namespace NM 
{
	namespace DataServices
	{
		using ::NM::ODB::OBJECTUID;

		/**
		*
		*
		*/
		IAdjacencyMatrix::IAdjacencyMatrix(::std::shared_ptr<CAdjacencyMatrix> adjMatrix):
			_pMatrix(adjMatrix)
		{
		}
		/**
		*
		*
		*/
		IAdjacencyMatrix::~IAdjacencyMatrix(void)
		{
		}
		/**
		*
		*
		*/
		void IAdjacencyMatrix::GetVertexEdges(OBJECTUID VertexID, ::std::vector<OBJECTUID> &edgelist)
		{
			_pMatrix->GetVertexEdges(VertexID, edgelist);
		}
		/**
		*
		*
		*/
		size_t IAdjacencyMatrix::GetVertexDegree(OBJECTUID VertexID)
		{
			return _pMatrix->GetVertexDegree(VertexID);
		}
		/**
		*
		*
		*/
		size_t IAdjacencyMatrix::GetVertexConnectedVerticies(OBJECTUID VertexID,::std::vector<OBJECTUID> &vertexlist)
		{
			return _pMatrix->GetVertexConnectedVerticies(VertexID, vertexlist);
		}
		/**
		*
		*
		*/
		size_t IAdjacencyMatrix::GetVertexConnectedEdgeCount(OBJECTUID srcVertexID, OBJECTUID dstVertexID)
		{
			return _pMatrix->GetVertexConnectedEdgeCount(srcVertexID, dstVertexID);
		}
		/**
		*
		*
		*/
		void IAdjacencyMatrix::GetVertexConnectedEdges(OBJECTUID srcVertexID, OBJECTUID dstVertexID, ::std::vector<OBJECTUID> &edgelist)
		{
			return _pMatrix->GetVertexConnectedEdges(srcVertexID, dstVertexID, edgelist);
		}
		/**
		*
		*
		*/
		void IAdjacencyMatrix::GetMatrixSize(size_t &rows, size_t &cols)
		{
			return _pMatrix->GetMatrixSize(rows, cols);
		}



//ns

	}
}