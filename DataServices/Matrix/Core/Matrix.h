#pragma once
#include <vector>
#include <assert.h>

namespace NM 
{
	namespace DataServices 
	{
		template <class T>
		class CObjectMatrix
		{
		public:
	
			CObjectMatrix():
				_rows(0),
				_cols(0), 
				_matrix(nullptr)
			{
				CreateMatrix(_rows, _cols);
			};

			/// Constructor
			/// Build a matrix of rows x cols.
			/// T defaultvalue is used when creating the matrix and when new rows or columns are added.
			CObjectMatrix(int rows, int cols, T defaultvalue):
				_rows(rows), 
				_cols(cols), 
				_matrix(nullptr),
				_defaultvalue(defaultvalue)
			{
				CreateMatrix(_rows, _cols);
			};

			virtual ~CObjectMatrix()
			{
				delete _matrix;
			};
	
	
			/// Get/Set value at row,col
			///
			T& operator()(unsigned int row, unsigned int col)
			{
				assert(row < _rows);
				assert(col < _cols);

				return _matrix->at(row).at(col);
			};

	
			/// removes all elements from the matrix
			///
			void erase()
			{
				delete _matrix;
				_rows = 0;
				_cols = 0;
				CreateMatrix(0,0);
			};

			/// clears/resets all values in every cell
			///
			void clear(T defaultvalue = _defaultvalue)
			{
				for(int r=0; r<_rows; ++r)
				{
					for(int c=0; c<_cols; ++c)
					{
						_matrix->at(r).at(c) = defaultvalue;
					}
				}
			};

	
			/// return the number of rows in the matrix.
			///
			unsigned int GetRowCount()
			{	
				return _rows;	
			}

	
			/// return the number of columns in the matrix
			///
			unsigned int GetColCount()
			{	
				return _cols;	
			}		

	
			/// Insert a new row into the matrix. 
			/// unsigned int insertbeforeindex; the one based index of where to insert the new row before
			int InsertRow(unsigned int insertbeforeindex = 0)
			{
				assert(insertbeforeindex <= _rows);

				td_cols row;
				for(unsigned int c=0; c<_cols; ++c)
				{
					row.push_back(_defaultvalue);		// add cell to row
				}

				_rows++;							// update internal counter
				if(insertbeforeindex == 0)
				{
					// add new row to matrix
					_matrix->push_back(row);
					// return index
					return _rows-1;			
				}
				else
				{
					// insert new row into matrix
					::std::vector<td_cols>::iterator it = _matrix->insert(_matrix->begin()+insertbeforeindex, row);
					// return index
					return it - _matrix->begin();
				}
				
				return -1;
			}
	
			/// Insert a new column into the matrix. 
			/// unsigned int insertbeforeindex; the one based index of where to insert the new column before
			/// All existing rows will have the new column added and the default value applied.
			int InsertColumn(unsigned int insertbeforeindex = 0)
			{
				assert(insertbeforeindex <= _cols);

					// for each existing row insert the col at the specified index
				if((insertbeforeindex == 0) || (_matrix->size() == 0))
				{
					// insertbeforeindex is 0, so append to end
					for(size_t t=0; t<_matrix->size(); t++)
					{
						_matrix->at(t).push_back(_defaultvalue);
					}
					_cols++;
					return _cols-1;		// return new index
				}
				else
				{
					// insert before..
					unsigned int index;
					for(size_t t=0; t<_matrix->size(); t++)
					{
						td_cols::iterator it = _matrix->at(t).insert(_matrix->at(t).begin()+insertbeforeindex, _defaultvalue);
						index = it - _matrix->at(t).begin();
					}
					_cols++;
					return index;

				}

				return 0;
			}


		private:
			typedef ::std::vector< T > td_cols;										
			::std::vector< td_cols >	*_matrix;

			unsigned int	_rows;		//!< Internal Class Row Count
			unsigned int	_cols;		//!< Internal Class Col Count

			T				_defaultvalue;

			void CreateMatrix(int nrow, int ncol)
			{
				_matrix = new ::std::vector< td_cols >;
		
				td_cols cols;

				for(int r=0; r<nrow; ++r)
				{
					for(int c=0; c<ncol; ++c)
					{
						cols.push_back(_defaultvalue);
					}
			
					_matrix->push_back(cols);
					cols.clear();
				}
			};
		};

// ns
	}
}

