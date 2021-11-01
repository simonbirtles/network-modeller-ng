#include "stdafx.h"
#include "NetworkViewSpatialHash.h"

#ifdef DEBUG
#include "IServiceRegistry.h"							// registry interface
#include "Interfaces\IObjectDatabase.h"					// for client interface to core database 
using ::NM::ODB::OBJECTUID;
using ::NM::ODB::UPVALUE;
using ::NM::ODB::INVALID_OBJECT_UID;
using ::NM::ODB::real_int;
using ::NM::ODB::real_uid;
using ::NM::ODB::real_string;
using ::NM::ODB::real_bool;

extern ::NM::Registry::IServiceRegistry* reg;
#endif 


namespace NM
{
	namespace NetGraph
	{
		/**
		*
		*
		*
		*/
		NetworkViewSpatialHash::NetworkViewSpatialHash(int spatialGridSize):
			_spatialGridSize(spatialGridSize)
		{

#ifdef DEBUG
			// if the global registry is not available we are in trouble
			if (!reg)
				throw ::std::runtime_error("Application Registry Not Available, Cannot Continue.");

			// Get The Database Handle and save
			_odb.reset(reinterpret_cast<NM::ODB::IObjectDatabase*>(reg->GetClientInterface(L"ODB")));
			if (!_odb)
				throw ::std::runtime_error("Database Not Available, Cannot Continue.");
#endif
		}
		/**
		*
		*
		*
		*/
		NetworkViewSpatialHash::~NetworkViewSpatialHash()
		{
		}
		/**
		*
		*
		*
		*/
		void NetworkViewSpatialHash::InsertSpatialHashNode(const ::NM::ODB::OBJECTUID& objectUID, ::std::set<::std::pair<int, int>>& pts)
		{
			//::NM::ODB::OBJECTUID objectUID = pLink->GetLinkUID();
			assert(objectUID != ::NM::ODB::INVALID_OBJECT_UID);
			assert(!pts.empty());

			// use reverse reference lookup to check validity i.e UID -> Cells (4x)
			UID_CELL_MAP::iterator uidcellit = _mUIDCell.find(objectUID);
			if (uidcellit != _mUIDCell.end())
			{
				return;
				// already a reference for this object UID
				throw ::std::logic_error("\nSpatial Hash Index already has this object - this should not happen!");
			}
			
		/*	::std::set<::std::pair<int, int>>::iterator pit = pts.begin();
			while (pit != pts.end())
			{
				::std::wstring text = L"\n Cell (" + ::std::to_wstring(pit->first) + L"," + ::std::to_wstring(pit->second) + L")";
				OutputDebugString(text.c_str());
				++pit;
			}*/

			// create spatial hash entry has for this node
			::std::set<CELLXY>::iterator vit = pts.begin();
			while (vit != pts.end())
			{
				// see if cell exists in unordered_map
				CELLMAP::iterator it = _cellHashMap.find(*vit);
				// if we have current cell
				if (it != _cellHashMap.end())
				{
					// insert objectid into the second element of the
					// unordered map, first element is grid (x,y), 
					// sets dont allow dups, so for 4 points of a image for same
					// object, it wont have 4 entries if in the same grid cell.
					it->second.insert(objectUID);
				}
				else
				{
					// no cell (grid) for this grid(x,y)
					// create a new one and add the objectuid
					VUID v;
					v.insert(objectUID);
					_cellHashMap[*vit] = v;
				}
				++vit;
			}

			// add to reverse lookup map
			_mUIDCell[objectUID] = pts;

			return;
		}
		/**
		*
		*
		*
		*/
		void NetworkViewSpatialHash::UpdateSpatialHashNode(const ::NM::ODB::OBJECTUID& objectUID, ::std::set<::std::pair<int, int>>& pts)
		{
			DeleteSpatialHashNode(objectUID);
			InsertSpatialHashNode(objectUID, pts);
			return;
		}
		/**
		* DeleteSpatialHashNode
		*
		*
		*/
		void NetworkViewSpatialHash::DeleteSpatialHashNode(const ::NM::ODB::OBJECTUID& objectUID)
		{
			// use reverse reference lookup to find cells i.e UID -> Cells (4x)
			UID_CELL_MAP::iterator uidcellit = _mUIDCell.find(objectUID);
			if (uidcellit == _mUIDCell.end())
			{
				return; // object does not exist
			}

			//  delete this object uid from each cell it exists in
			::std::set<CELLXY>::iterator cellit = uidcellit->second.begin(); // get iterator to vector (uidcellit->second) of cells this objectuid is in
																			 // for each cell the ojectuid is in
			while (cellit != uidcellit->second.end())
			{
				// get uid grid (x,y)
				int x = cellit->first;
				int y = cellit->second;

				// find grid(x,y)
				CELLMAP::iterator cellmapit = _cellHashMap.find(::std::make_pair(x, y));
				if (cellmapit != _cellHashMap.end())
				{
					// this is the ::std::set of objectuids in this cell
					VUID::iterator vit = cellmapit->second.find(objectUID);
					while (vit != cellmapit->second.end())
					{
						cellmapit->second.erase(vit);
						vit = cellmapit->second.find(objectUID);
					}

				}
				++cellit;
			}

			// delete reverse reference
			_mUIDCell.erase(uidcellit);

			return;
		}
		/**
		* IsCaretOnObject
		*
		* Takes a client window x,y co-ord and uses spatial hash/index to identify the potential nodes
		* in the virtual grid the point is in, then for each potential, the node class instance is
		* then asked to validate if the point is in its bitmap. A final list of potential nodes
		* (ones which all match the point.. overlapping incl.) are returned
		*/
		::std::vector<::NM::ODB::OBJECTUID> NetworkViewSpatialHash::GetPotentialObjects(POINT &clientWindowPoint)
		{
			::std::vector<::NM::ODB::OBJECTUID> potentialLinks;
		
			CELLXY clickpt = ::std::make_pair((int)abs((float)(clientWindowPoint.x) / _spatialGridSize), (int)abs((float)(clientWindowPoint.y) / _spatialGridSize));

#ifdef DEBUG
			::std::wstring strPt = L"(" + ::std::to_wstring(clickpt.first) + L":" + ::std::to_wstring(clickpt.second) + L")";
			CString outText;
			outText.Format(L"\n\nClick detected in Grid: Cell %s for points: %d,%d", strPt.c_str(), clientWindowPoint.x, clientWindowPoint.y);
			//OutputDebugString(L"\n\nClick detected in Grid: Cell");
			//OutputDebugString(strPt.c_str());
			OutputDebugString(outText);
#endif 
			CELLMAP::iterator it = _cellHashMap.find(clickpt);
			if (it != _cellHashMap.end())
			{
				for each(::NM::ODB::OBJECTUID objectid in it->second)
				{
					potentialLinks.push_back(objectid);
				}
			}
#ifdef DEBUG
			if (potentialLinks.size() > 0)
			{
				for each(::NM::ODB::OBJECTUID objectid in potentialLinks)
				{
					OutputDebugString(L"\nPotential Link\t");
					if(_odb->IsValidObjectUID(objectid))
						OutputDebugString(_odb->GetValue(objectid, L"shortname")->Get<::NM::ODB::ODBWString>().c_str());
					else
						OutputDebugString(L"Internal Link");

				}
			}
			else
			{
				OutputDebugString(L"\nNo link found at click point");
			}
#endif // DEBUG
			return potentialLinks;
		}

		// ns
	}
}
