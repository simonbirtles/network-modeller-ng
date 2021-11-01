#pragma once
#include "interfaces\ObjectDatabaseDefines.h"
#include <unordered_map>
#include <map>
#include <set>
#include <memory>

namespace NM
{
#ifdef DEBUG
	namespace ODB
	{
		class IObjectDatabase;
	}
#endif 

	namespace NetGraph
	{
		class Link;

		class NetworkViewSpatialHash
		{
		public:
			// for map
			struct compareGuid
			{
				bool operator()(const ::NM::ODB::ODBUID& guid1, const::NM::ODB::ODBUID& guid2) const
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

			NetworkViewSpatialHash(int spatialGridSize);
			~NetworkViewSpatialHash();

			// Spaital Hash
			::std::vector<::NM::ODB::OBJECTUID> GetPotentialObjects(POINT &clientWindowPoint);
			void InsertSpatialHashNode(const ::NM::ODB::OBJECTUID&, ::std::set<::std::pair<int, int>>& pts);
			void UpdateSpatialHashNode(const ::NM::ODB::OBJECTUID&, ::std::set<::std::pair<int, int>>& pts);
			void DeleteSpatialHashNode(const ::NM::ODB::OBJECTUID&);

			int GetSpatialGridSize() { return _spatialGridSize; };

		private:
			int _spatialGridSize;
#ifdef DEBUG
			::std::unique_ptr<::NM::ODB::IObjectDatabase>			 _odb;					// interface to the database
#endif

			typedef std::pair<int, int> CELLXY;														// Cell reference e.g (1,3)
			typedef ::std::set<::NM::ODB::OBJECTUID, compareGuid> VUID;								// set of ObjectUIDs in a cell CELLXY 
			typedef ::std::map<::NM::ODB::OBJECTUID, ::std::set<CELLXY>, compareGuid> UID_CELL_MAP;			// maps ObjectUID -> Cells

			UID_CELL_MAP _mUIDCell;

			class pairHash {
			public:
				size_t operator()(const CELLXY &k) const
				{
					return k.first * 100 + k.second;
				}
			};
			typedef ::std::unordered_map<CELLXY, VUID, pairHash> CELLMAP;
			CELLMAP _cellHashMap;


		};

		// ns
	}
}

