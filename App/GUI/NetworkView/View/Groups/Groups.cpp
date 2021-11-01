#include "stdafx.h"
#include "Groups.h"
#include "IServiceRegistry.h"							// registry interface
#include "Interfaces\IObjectDatabase.h"					// for client interface to core database 
#include "Interfaces\IAttribute.h"						// interface to an attribute copy

extern NM::Registry::IServiceRegistry* reg;

//
// TODO: CNetworkView header has 'Group group' declared, need to chg to pointer and instantiate manually

//

/*
*
*
*/
 Groups::Groups() :
     // root group = ::NM::ODB::INVALID_OBJECT_UID
     _currentGroupUID(::NM::ODB::INVALID_OBJECT_UID)
{

     // if the global registry is not available we are in trouble
     if (!reg)
         throw ::std::runtime_error("Application Registry Not Available, Cannot Continue.");

     // Get The Database Handle and save
     _odb.reset(reinterpret_cast<NM::ODB::IObjectDatabase*>(reg->GetClientInterface(L"ODB")));
     if (!_odb)
         throw ::std::runtime_error("Database Not Available, Cannot Continue.");

     // Get the handle to the database update cache
     _updateCache.reset(reinterpret_cast<::NM::ODB::IDatabaseUpdate*>(reg->GetClientInterface(L"ObjectUpdateCache")));
     if (!_updateCache)
         throw ::std::runtime_error("Database UpdateCache Not Available, Cannot Continue.");

     RefreshGroups();

     ::std::vector<::std::wstring> attr_list;
     attr_list.push_back(L"create");
     attr_list.push_back(L"delete");
     _updateQ = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Group, attr_list);

     
}
 /*
*
*
*/
 Groups::~Groups()
{
}
 /*
 *
 *
 */
 void Groups::RefreshGroups()
 {
     ::std::wstring table = L"grouptable";
     ::NM::ODB::OBJECTUID objectUID = _odb->GetFirstObject(table);

     while (objectUID != ::NM::ODB::INVALID_OBJECT_UID)
     {
         ::NM::ODB::real_string groupName = _odb->GetValue(objectUID, L"shortname")->Get<::NM::ODB::ODBWString>();
         ::NM::ODB::OBJECTUID parentUID = _odb->GetValue(objectUID, L"group")->Get<::NM::ODB::ODBUID>();
         AddGroup(objectUID, parentUID, groupName);
         // next object
         objectUID = _odb->GetNextObject(objectUID);
     }
     ValidateGroupTree();
     OutputDebugString(L"\nGroup refresh complete.\n");
 }
 /*
*
*
*/
 void Groups::AddGroup(::NM::ODB::OBJECTUID objectUID, ::NM::ODB::OBJECTUID parentUID, std::wstring groupName)
 {
     CString out;
     out.Format(L"\nGroup Add: %s", groupName.c_str());
     OutputDebugString(out);

     SPGROUP grp = ::std::make_shared<GROUP>(objectUID, parentUID, groupName);
     _groups.insert({ objectUID, grp });
   
 }
 /*
 *
 * Sets child refereneces
 * Removes any invalid references / groups where parents or children dont exist.
 */
 BOOL Groups::ValidateGroupTree()
 {
     GROUP::UIDSET invalidGroups;

     UID_GROUP_MAP::iterator groupIt = _groups.begin();
     while(groupIt != _groups.end())
     {
         // if not a root group
         ::NM::ODB::OBJECTUID parentUID = groupIt->second->GetParentGroup();
         if (parentUID != ::NM::ODB::INVALID_OBJECT_UID)
         {
             // get parent id and if not root then check we have the 
             // group and add this uid as a child to the parent id.
             // if not found add to invalid groups set and delete later
             UID_GROUP_MAP::iterator groupParentIt = _groups.find(parentUID);
             if (groupParentIt != _groups.end())
             {
                 groupParentIt->second->AddChildGroup(groupIt->first);
             }
             else
             {
                 invalidGroups.insert(groupIt->first);
             }
         }
         ++groupIt;
     }

     // remove any invalid groups
     ::std::set<::NM::ODB::OBJECTUID>::iterator invalidGroupsIt = invalidGroups.begin();
     while (invalidGroupsIt != invalidGroups.end())
     {
         CString out;
         out.Format(L"\nRemoving invalid groups due to having invalid parent.");
         OutputDebugString(out);
         DeleteGroup(*invalidGroupsIt);
         ++invalidGroupsIt;
     }

     OutputDebugString(L"\nGroup validation complete.");
     return TRUE;
 }
 /*
*
*
*/
 BOOL Groups::DeleteGroup(::NM::ODB::OBJECTUID objectUID)
 {
     // find group 
     UID_GROUP_MAP::iterator parentGroupIt = _groups.find(objectUID);
     if (parentGroupIt == _groups.end())
         return TRUE;

     // get all group children
     GROUP::UIDSET children = parentGroupIt->second->GetChildren();
     // loop through child tree and delete all children 
     GROUP::UIDSET::iterator childIt = children.begin();
     while (childIt != children.end())
     {
         DeleteGroup(*childIt);
         ++childIt;
     }

     CString out;
     out.Format(L"\nDeleting group '%s'.", parentGroupIt->second->GetGroupName().c_str());
     OutputDebugString(out);
     // delete group
     _groups.erase(objectUID);
     return TRUE;
 }

  /*
*
*
*/
 ::NM::ODB::OBJECTUID Groups::GetActiveGroup()
 {
     return _currentGroupUID;
 }
 /*
 *
 *
 */
 BOOL Groups::SetActiveGroup(::NM::ODB::OBJECTUID groupUID)
 {
     if (GetGroup(groupUID) == nullptr)
         return FALSE;
     _currentGroupUID = groupUID;
     return TRUE;
 }
/*
*
*
*/
 BOOL Groups::SetGroupName(::NM::ODB::OBJECTUID groupUID, CString groupName)
 {
     SPGROUP cGroup = GetGroup(groupUID);
     ASSERT(cGroup != nullptr);
     cGroup->SetGroupName(groupName);
     return TRUE;
 }
 /*
 *
 *
 */
 void Groups::SetTranslation(int x, int y)
 {
     SPGROUP cGroup = GetGroup(_currentGroupUID);
     ASSERT(cGroup != nullptr);
     cGroup->SetTranslation(x, y);
 }
 /*
 *
 *
 */
 void Groups::SetScale(float scaleX, float scaleY)
 {
     SPGROUP cGroup = GetGroup(_currentGroupUID);
     ASSERT(cGroup != nullptr);
     cGroup->SetScale(scaleX, scaleY);
 }
 /*
*
*
*/
 ::std::wstring Groups::GetGroupName(::NM::ODB::OBJECTUID groupUID)
 {
     SPGROUP cGroup = GetGroup(groupUID);
     ASSERT(cGroup != nullptr);
     if (cGroup == nullptr)
         return L"";
     return cGroup->GetGroupName();
 }
 /*
 *
 *
 */
 void Groups::GetTranslation(int& x, int& y)
 {
     SPGROUP cGroup = GetGroup(_currentGroupUID);
     ASSERT(cGroup != nullptr);
     cGroup->GetTranslation(x, y);
 }
 /*
 *
 *
 */
 void Groups::GetScale(float& scaleX, float& scaleY)
 {
     SPGROUP cGroup = GetGroup(_currentGroupUID);
     ASSERT(cGroup != nullptr);
     cGroup->GetScale(scaleX, scaleY);
 }
 /*
 *
 *
 */
 Groups::SPGROUP Groups::GetGroup(::NM::ODB::OBJECTUID objectUID)
 {
     UID_GROUP_MAP::iterator it = _groups.find(objectUID);
     if (it == _groups.end())
         return nullptr;
     return it->second;
 }
 /*
 *
 *
 */
 void Groups::DatabaseUpdate()
 {

 }