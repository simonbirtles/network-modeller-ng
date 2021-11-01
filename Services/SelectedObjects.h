#pragma once
#include "SelectedObjectsDefines.h"
#include "..\ObjectDatabase\Interfaces\DatabaseObserver.h"					// base class for client listener (dervive from for update calls)
#include "..\ObjectDatabase\Interfaces\IDatabaseUpdate.h"	
#include <vector>

namespace NM
{
	class SelectedObjectsClientObs;

	class SelectedObjects:
		public ::NM::ODB::CDatabaseObserver
	{
	public:

		SelectedObjects();
		~SelectedObjects();

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
		void				DatabaseUpdate();



	private:		
		OBJECTVECTOR	_selectedObjects;
		bool			_multiselect;
		typedef ::std::vector<SelectedObjectsClientObs*> OBSERVERS;
		OBSERVERS		_clientObs;
		bool			_delayUpdates;

		void			SendUpdate();
		void			SendMultiSelectChangeUpdate();

		::std::unique_ptr<::NM::ODB::IDatabaseUpdate> _updateCache;
		::NM::ODB::IDatabaseUpdate::UpdateQueueHandle	 _updateQ;
	};

}
