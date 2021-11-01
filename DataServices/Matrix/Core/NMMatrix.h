#pragma once
#include "ObjectDatabaseDefines.h"
#include <vector>
#include <map>

namespace NM
{
	namespace DataServices
	{
		template <class T> class CObjectMatrix;

		class NMMatrix	
		{
		public:

			enum EDGETYPE { EDGE_DIRECTED, EDGE_UNDIRECTED };

			NMMatrix();
			virtual ~NMMatrix();

			void CreateVertex(::NM::ODB::OBJECTUID vertexID);
			void CreateEdge(::NM::ODB::OBJECTUID VertexSrcID, ::NM::ODB::OBJECTUID VertexDstID, ::NM::ODB::OBJECTUID eid, enum EDGETYPE edgetype);

			void DeleteVertex(::NM::ODB::OBJECTUID VertexID);			// not sure if we need this
			void DeleteEdge(::NM::ODB::OBJECTUID VertexSrcID, ::NM::ODB::OBJECTUID VertexDstID, ::NM::ODB::OBJECTUID eid, enum EDGETYPE edgetype);
			void DeleteAllEdges(::NM::ODB::OBJECTUID VertexSrcID, ::NM::ODB::OBJECTUID VertexDstID, enum EDGETYPE edgetype);

			void Clear();

			size_t	GetVertexConnectedVerticies(::NM::ODB::OBJECTUID VertexID, ::std::vector<::NM::ODB::OBJECTUID> &vertexlist) ;
			size_t	GetVertexConnectedEdgeCount(::NM::ODB::OBJECTUID srcVertexID, ::NM::ODB::OBJECTUID dstVertexID) ;
			void	GetVertexConnectedEdges(::NM::ODB::OBJECTUID srcVertexID, ::NM::ODB::OBJECTUID dstVertexID, ::std::vector<::NM::ODB::OBJECTUID> &edgelist) ;
			void	GetVertexEdges(::NM::ODB::OBJECTUID VertexID, ::std::vector<::NM::ODB::OBJECTUID> &edgelist) ;
			size_t	GetVertexDegree(::NM::ODB::OBJECTUID VertexID) ;
			void	GetMatrixSize(size_t &rows, size_t &cols) ;

			// testing
			void	OutputMatrixToConsole();

			typedef ::std::vector<::NM::ODB::OBJECTUID> edgelist;
			typedef ::std::vector<::NM::ODB::OBJECTUID> vertexlist;


		private:

			struct compareGuid
			{
				bool operator()(const NM::ODB::OBJECTUID& guid1, const NM::ODB::OBJECTUID& guid2) const
				{
					if (guid1.Data1 != guid2.Data1) {
						return guid1.Data1 < guid2.Data1;
					}
					if (guid1.Data2 != guid2.Data2) {
						return guid1.Data2 < guid2.Data2;
					}
					if (guid1.Data3 != guid2.Data3) {
						return guid1.Data3 < guid2.Data3;
					}
					for (int i = 0;i<8;i++) {
						if (guid1.Data4[i] != guid2.Data4[i]) {
							return guid1.Data4[i] < guid2.Data4[i];
						}
					}
					return false;
				}
			};

			CObjectMatrix<edgelist*> *_matrix;

			typedef ::NM::ODB::OBJECTUID vertex_uid;
			typedef int matrix_uid;
			typedef ::std::map<vertex_uid, matrix_uid, compareGuid> VertexToMatrixMap;
			typedef ::std::map<matrix_uid, vertex_uid> MatrixToVertexMap;
			VertexToMatrixMap _vertex_uid_to_matrixid_map;
			MatrixToVertexMap _matrixid_to_vertex_uid_map;

			void AddVertexIDMap(vertex_uid, matrix_uid);
			void DeleteVertexIDMap(vertex_uid, matrix_uid);
			::NM::ODB::OBJECTUID GetVertexUIDFromMatrixID(matrix_uid);
			matrix_uid GetMatrixIDFromVertexUID(vertex_uid);

		};

	}
}
