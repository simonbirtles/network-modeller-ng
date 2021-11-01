#pragma once
#include "ObjectDatabaseDefines.h"
#include "DatabaseObserver.h"
#include "IDatabaseUpdate.h"
#include "..\Core\NMMatrix.h"

#include <memory>

namespace NM 
{
	namespace ODB
	{
		class IObjectDatabase;
	}
	namespace DataServices 
	{
		

		class CAdjacencyMatrix :
			public ::NM::ODB::CDatabaseObserver, 
			public NMMatrix
		{
		public:
			CAdjacencyMatrix(void);
			~CAdjacencyMatrix(void);		

			void DatabaseUpdate();

		private:			
			::NM::ODB::IDatabaseUpdate::UpdateQueueHandle _pUpdateQueue;
			::std::unique_ptr<::NM::ODB::IObjectDatabase> _odb;

			void RebuildMatrix();

	
		};

		// ns
	}
}

