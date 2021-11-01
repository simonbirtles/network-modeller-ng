#pragma once
#include "SelectedObjectsDefines.h"


namespace NM
{
	class SelectedObjects;
	class SelectedObjectsClientObs;

	class ISelectedObjects
	{
	public:
		

		ISelectedObjects(SelectedObjects** selectedObjects);
		~ISelectedObjects();
		
		void				Select(NM::ODB::OBJECTUID);
		void				Select(OBJECTVECTOR&);
		void				Deselect(NM::ODB::OBJECTUID);
		void				Deselect(OBJECTVECTOR&);
		void				Clear();
		bool				IsSelected(NM::ODB::OBJECTUID);
		size_t				GetSelected(OBJECTVECTOR&);
		size_t				SelectedCount();
		NM::ODB::OBJECTUID	GetSelectedPos(SelectedObjectPosition pos);
		void				SetMultiSelect(bool bMultiSelect);
		bool				IsMultiSelect();
		bool				RegisterClientObserver(SelectedObjectsClientObs* client);
		bool				DeRegisterClientObserver(SelectedObjectsClientObs* client);

	private:
		SelectedObjects** _selectedObjects;
	};

}

