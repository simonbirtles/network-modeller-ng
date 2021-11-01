#include "stdafx.h"
#include "NMMatrix.h"
#include "matrix.h"


// testing console output /////////////////////////////
#include <stdio.h>
#include <iostream>
#include <io.h>
#include <fcntl.h>
//////////////////////////////////////////////////////

namespace NM
{
	namespace DataServices
	{
		using ::NM::ODB::OBJECTUID;

		NMMatrix::NMMatrix() :
			_matrix(new CObjectMatrix<edgelist*>(0, 0, nullptr))
		{
		}


		NMMatrix::~NMMatrix()
		{

			// delete pointers to vectors in each r,c
			size_t cols = _matrix->GetColCount();
			for (size_t t = 0; t<cols; ++t)
			{
				DeleteVertex(GetVertexUIDFromMatrixID(t));
			}
			delete _matrix;
		}
		/**
		*
		*
		*
		*/
		void NMMatrix::Clear()
		{
			// delete pointers to vectors in each r,c
			//size_t cols = _matrix->GetColCount();
			//for(size_t t=0; t<cols; ++t)
			//{
			//	DeleteVertex(t);
			//}

			VertexToMatrixMap::iterator it = _vertex_uid_to_matrixid_map.begin();
			while (it != _vertex_uid_to_matrixid_map.end())
			{
				DeleteVertex(it->first);
				it = _vertex_uid_to_matrixid_map.begin();		// rest as the DeleteVertex func will delete this from the map we are using and invalidate iterator.
				++it;
			}

			// erase matrix back to 0,0
			_matrix->erase();
			_vertex_uid_to_matrixid_map.clear();
			_matrixid_to_vertex_uid_map.clear();

		}
		/**
		*
		*
		*
		*/
		void NMMatrix::CreateVertex(OBJECTUID vertexID)
		{

			//size_t highestVID = _matrix->GetColCount()-1;

			size_t matrixid = GetMatrixIDFromVertexUID(vertexID);
			if (matrixid != -1)
			{
				// vertex already exists in matrix
				return;
			}

			// insert at end of matrix - order not important ?
			size_t col_id = _matrix->InsertColumn(0);
			size_t row_id = _matrix->InsertRow(0);

			// check matrix still square
			assert(col_id == row_id);

			// add new vertexid to map, with next matrix id
			AddVertexIDMap(vertexID, col_id);

			// add it in, but make sure we have all inbetween added too incase id has jumped, we use ID for row, col...
			//for(size_t i=highestVID+1; i<=vertexID; ++i)
			//{
			//	_matrix->InsertColumn(0);
			//	_matrix->InsertRow(0);
			//	//(*_matrix)(i, i) = nullptr;
			//}

#ifdef _DEBUG
			size_t cols = _matrix->GetColCount();
			size_t rows = _matrix->GetRowCount();
			assert(cols == rows);
#endif

			return;
		}
		/**
		*
		*
		*
		*/
		void NMMatrix::CreateEdge(OBJECTUID VertexSrcID, OBJECTUID VertexDstID, OBJECTUID eid, enum EDGETYPE edgetype)
		{
			int src_matrix_id = GetMatrixIDFromVertexUID(VertexSrcID);
			int dst_matrix_id = GetMatrixIDFromVertexUID(VertexDstID);

			if ((src_matrix_id == -1) ||
				(dst_matrix_id == -1))
			{
				// either src or dst vertex does not exist
				// log ?debug log
				return;
			}

			// if no current edges exist
			if ((*_matrix)(src_matrix_id, dst_matrix_id) == nullptr)
			{
				(*_matrix)(src_matrix_id, dst_matrix_id) = new edgelist;
			}

			// Add Edge to list
			((*_matrix)(src_matrix_id, dst_matrix_id))->push_back(eid);

			// if undirected, copy the pointer into the other diagonal of the matrix, otherwise a new will be created when the other direction is called on CreateEdge
			if (edgetype == EDGE_UNDIRECTED)
			{
				if ((*_matrix)(dst_matrix_id, src_matrix_id) == nullptr)
				{
					(*_matrix)(dst_matrix_id, src_matrix_id) = ((*_matrix)(src_matrix_id, dst_matrix_id));
				}
			}

			return;
		}
		/**
		*
		*
		*
		*/
		void NMMatrix::DeleteVertex(OBJECTUID VertexID)			// not sure if we need this
		{
			int matrix_id = GetMatrixIDFromVertexUID(VertexID);
			if (matrix_id == -1)
			{
				return;		//not found
			}
			size_t cols = _matrix->GetColCount();

#ifdef _DEBUG
			size_t rows = _matrix->GetRowCount();
			assert(cols == rows);
#endif

			for (size_t t = 0; t<cols; ++t)
			{
				if ((*_matrix)(matrix_id, t) == (*_matrix)(t, matrix_id))				// undirected edge
				{
					delete (*_matrix)(matrix_id, t);
					(*_matrix)(matrix_id, t) = nullptr;
					(*_matrix)(t, matrix_id) = nullptr;
				}
				else																	// maybe directed
				{
					if ((*_matrix)(matrix_id, t) != nullptr)
					{
						delete (*_matrix)(matrix_id, t);
						(*_matrix)(matrix_id, t) = nullptr;
					}

					if ((*_matrix)(t, matrix_id) != nullptr)
					{
						delete (*_matrix)(t, matrix_id);
						(*_matrix)(t, matrix_id) = nullptr;
					}
				}
			}
			DeleteVertexIDMap(VertexID, matrix_id);
		}
		/**
		*
		*
		*
		*/
		void NMMatrix::DeleteEdge(OBJECTUID VertexSrcID, OBJECTUID VertexDstID, OBJECTUID eid, enum EDGETYPE edgetype)
		{
			size_t src_matrix_id = GetMatrixIDFromVertexUID(VertexSrcID);
			size_t dst_matrix_id = GetMatrixIDFromVertexUID(VertexDstID);
			if ((src_matrix_id == -1) ||
				(dst_matrix_id == -1))
			{
				// either src or dst vertex does not exist
				// log ?debug log
				return;
			}

			edgelist::iterator it;

			it = ::std::find((*_matrix)(src_matrix_id, dst_matrix_id)->begin(),
				(*_matrix)(src_matrix_id, dst_matrix_id)->end(),
				eid
				);

			if (it != (*_matrix)(src_matrix_id, dst_matrix_id)->end())
			{
				(*_matrix)(src_matrix_id, dst_matrix_id)->erase(it);
			}


			if (edgetype == EDGE_UNDIRECTED)
			{
				it = (*_matrix)(dst_matrix_id, src_matrix_id)->end();

				it = ::std::find((*_matrix)(dst_matrix_id, src_matrix_id)->begin(),
					(*_matrix)(dst_matrix_id, src_matrix_id)->end(),
					eid
					);

				if (it != (*_matrix)(dst_matrix_id, src_matrix_id)->end())
				{
					(*_matrix)(dst_matrix_id, src_matrix_id)->erase(it);
				}

			}
			return;
		}
		/**
		*
		*
		*
		*/
		void NMMatrix::DeleteAllEdges(OBJECTUID VertexSrcID, OBJECTUID VertexDstID, enum EDGETYPE edgetype)
		{
			int src_matrix_id = GetMatrixIDFromVertexUID(VertexSrcID);
			int dst_matrix_id = GetMatrixIDFromVertexUID(VertexDstID);
			if (src_matrix_id == -1) { return; } // does not exist
			if (dst_matrix_id == -1) { return; } // does not exist

			((*_matrix)(src_matrix_id, dst_matrix_id))->clear();
			delete ((*_matrix)(src_matrix_id, dst_matrix_id));
			((*_matrix)(src_matrix_id, dst_matrix_id)) = nullptr;

			if (edgetype == EDGE_UNDIRECTED)
			{
				((*_matrix)(dst_matrix_id, src_matrix_id)) = nullptr;
			}
		}
		/**
		*
		*
		*
		*/
		//size_t NMMatrix::GetVertexDegree(size_t VertexID)
		//{
		//	size_t matrix_id = GetMatrixIDFromVertexUID( VertexID );
		//	if( matrix_id == -1 )
		//	{
		//		// does not exist
		//		return 0;
		//	}
		//
		//	size_t degree = 0;
		//	size_t cols = _matrix->GetColCount();
		//
		//	for(size_t t=0; t<cols; ++t)
		//	{
		//		if( (*_matrix)(matrix_id, t) == (*_matrix)(t, matrix_id) && 				// undirected edge so r,c = c,r
		//				( (*_matrix)(matrix_id, t) != nullptr) )
		//		{
		//					 ++degree;
		//		}
		//		else																	// maybe directed
		//		{
		//			if( (*_matrix)(matrix_id, t) != nullptr)
		//			{
		//				++degree;
		//			}
		//
		//			if( (*_matrix)(t, matrix_id) != nullptr)
		//			{
		//				++degree;
		//			}
		//		}
		//	}	
		//
		//	return degree;
		//}
		/**
		*
		*
		*
		*/
		size_t NMMatrix::GetVertexDegree(OBJECTUID VertexID)
		{
			size_t edgecount = 0;
			int matrix_id = GetMatrixIDFromVertexUID(VertexID);
			if (matrix_id == -1) { return 0; } // does not exist

			size_t cols = _matrix->GetColCount();
			for (size_t t = 0; t<cols; ++t)
			{
				if ((*_matrix)(matrix_id, t) == (*_matrix)(t, matrix_id) && 				// undirected edge so r,c = c,r
					((*_matrix)(matrix_id, t) != nullptr))
				{

					edgecount += ((*_matrix)(t, matrix_id))->size();
				}
				else																	// maybe directed
				{

					if ((*_matrix)(matrix_id, t) != nullptr)
					{
						edgecount += ((*_matrix)(matrix_id, t))->size();
					}


					if ((*_matrix)(t, matrix_id) != nullptr)
					{
						edgecount += ((*_matrix)(t, matrix_id))->size();
					}
				}
			}

			return edgecount;
		}
		/**
		*
		*
		*
		*/
		void NMMatrix::GetVertexConnectedEdges(OBJECTUID srcVertexID, OBJECTUID dstVertexID, ::std::vector<OBJECTUID> &edgelist)
		{
			int src_matrix_id = GetMatrixIDFromVertexUID(srcVertexID);
			int dst_matrix_id = GetMatrixIDFromVertexUID(dstVertexID);
			if (src_matrix_id == -1) { return; } // does not exist
			if (dst_matrix_id == -1) { return; } // does not exist

			edgelist.clear();
			if (((*_matrix)(src_matrix_id, dst_matrix_id)) != nullptr)
			{
				edgelist = *((*_matrix)(src_matrix_id, dst_matrix_id));
			}
			return;
		}
		/**
		*
		*
		*
		*/
		size_t NMMatrix::GetVertexConnectedEdgeCount(OBJECTUID srcVertexID, OBJECTUID dstVertexID)
		{
			int src_matrix_id = GetMatrixIDFromVertexUID(srcVertexID);
			int dst_matrix_id = GetMatrixIDFromVertexUID(dstVertexID);
			if (src_matrix_id == -1) { return 0; } // does not exist
			if (dst_matrix_id == -1) { return 0; } // does not exist

			if ((*_matrix)(src_matrix_id, dst_matrix_id) != nullptr)
			{
				return ((*_matrix)(src_matrix_id, dst_matrix_id))->size();
			}

			return 0;
		}
		/**
		*
		*
		*
		*/
		// need to add directed code, currently undirected
		size_t NMMatrix::GetVertexConnectedVerticies(OBJECTUID VertexID, ::std::vector<OBJECTUID> &vertexlist)
		{
			int src_matrix_id = GetMatrixIDFromVertexUID(VertexID);
			if (src_matrix_id == -1) { return 0; } // does not exist

			size_t msize = _matrix->GetColCount();

			for (size_t col = 0;col<msize; ++col)
			{
				if ((*_matrix)(src_matrix_id, col) != nullptr)
				{
					if ((*_matrix)(src_matrix_id, col)->size() != 0)
					{
						vertexlist.push_back(GetVertexUIDFromMatrixID(col));
					}
				}
			}
			return vertexlist.size();
		}
		/**
		*
		*
		*
		*/
		void NMMatrix::GetVertexEdges(OBJECTUID VertexID, ::std::vector<OBJECTUID> &edgelist)
		{
			size_t matrix_id = GetMatrixIDFromVertexUID(VertexID);
			if (matrix_id == -1) { return; } // does not exist

			edgelist.clear();

			size_t cols = _matrix->GetColCount();

			for (size_t t = 0; t<cols; ++t)
			{

				if ((*_matrix)(matrix_id, t) == (*_matrix)(t, matrix_id) && 				// undirected edge so r,c = c,r
					((*_matrix)(matrix_id, t) != nullptr))
				{

					edgelist.insert(edgelist.end(),
						((*_matrix)(t, matrix_id))->begin(),
						((*_matrix)(t, matrix_id))->end()
						);
				}
				else																	// maybe directed
				{

					if ((*_matrix)(matrix_id, t) != nullptr)
					{
						edgelist.insert(edgelist.end(),
							((*_matrix)(matrix_id, t))->begin(),
							((*_matrix)(matrix_id, t))->end()
							);
					}


					if ((*_matrix)(t, matrix_id) != nullptr)
					{
						edgelist.insert(edgelist.end(),
							((*_matrix)(t, matrix_id))->begin(),
							((*_matrix)(t, matrix_id))->end()
							);
					}
				}
			}
		}
		/**
		*
		*
		*
		*/
		// create a new vertex to matrix id mapping
		void NMMatrix::AddVertexIDMap(vertex_uid vid, matrix_uid mid)
		{
			_vertex_uid_to_matrixid_map[vid] = mid;
			_matrixid_to_vertex_uid_map[mid] = vid;
		}
		/**
		*
		*
		*
		*/
		// delete a vertex to matrix id mapping
		void NMMatrix::DeleteVertexIDMap(vertex_uid vid, matrix_uid mid)
		{
			VertexToMatrixMap::iterator vmap_it = _vertex_uid_to_matrixid_map.find(vid);
			if (vmap_it != _vertex_uid_to_matrixid_map.end())
			{
				if (vmap_it->second == mid)		// check mapping is valid
				{
					_vertex_uid_to_matrixid_map.erase(vmap_it);
				}
			}

			MatrixToVertexMap::iterator mmap_it = _matrixid_to_vertex_uid_map.find(mid);
			if (mmap_it != _matrixid_to_vertex_uid_map.end())
			{
				if (mmap_it->second == vid)		// check mapping is valid
				{
					_matrixid_to_vertex_uid_map.erase(mmap_it);
				}
			}
		}
		/**
		*
		*
		*
		*/
		OBJECTUID NMMatrix::GetVertexUIDFromMatrixID(matrix_uid mid)
		{
			MatrixToVertexMap::iterator mmap_it = _matrixid_to_vertex_uid_map.find(mid);
			if (mmap_it != _matrixid_to_vertex_uid_map.end())
			{
				return mmap_it->second;		// return VertexID
			}
			else
			{
				return ::NM::ODB::INVALID_OBJECT_UID;					// not found
			}
		}
		/**
		*
		*
		*
		*/
		NMMatrix::matrix_uid NMMatrix::GetMatrixIDFromVertexUID(vertex_uid vid)
		{
			VertexToMatrixMap::iterator vmap_it = _vertex_uid_to_matrixid_map.find(vid);
			if (vmap_it != _vertex_uid_to_matrixid_map.end())
			{
				return vmap_it->second;	// return MatrixID
			}
			else
			{
				return -1;				// not found
			}
		}
		/**
		*
		*
		*
		*/
		void NMMatrix::GetMatrixSize(size_t &rows, size_t &cols)
		{
			rows = _matrix->GetRowCount();
			cols = _matrix->GetColCount();
		}




#pragma region testing
		// see includes at top
		void NMMatrix::OutputMatrixToConsole()
		{

			AllocConsole();

			HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
			int hCrt = _open_osfhandle((long)handle_out, _O_TEXT);
			FILE* hf_out = _fdopen(hCrt, "w");
			setvbuf(hf_out, NULL, _IONBF, 1);
			*stdout = *hf_out;

			HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
			hCrt = _open_osfhandle((long)handle_in, _O_TEXT);
			FILE* hf_in = _fdopen(hCrt, "r");
			setvbuf(hf_in, NULL, _IONBF, 128);
			*stdin = *hf_in;


			size_t cols = _matrix->GetColCount();
			size_t rows = _matrix->GetRowCount();

			for (size_t r = 0; r<rows; ++r)
			{
				for (size_t c = 0; c<cols; ++c)
				{
					if ((*_matrix)(r, c) != nullptr)
					{
						printf("%i ", ((*_matrix)(r, c))->size());
					}
					else
					{
						printf("0 ");
					}
				}

				printf("\n");

			}

			printf("\n\n\n\n");
			char a;
			::std::cin >> a;

			FreeConsole();

			return;
		}

#pragma endregion testing



	}
}