#pragma once
#include "..\ServiceRegistry\ServiceBase.h"

namespace NM
{
	namespace Serial
	{
		class ISerial;
	}
	
	namespace ODB
	{
		class CObjectDatabase;
		class IDatabaseUpdate;
		class GroupManager;


		class IObjectDatabaseService : public Registry::ServiceBase
		{
		public:
			IObjectDatabaseService(void);
			~IObjectDatabaseService(void);

			// pure virtuals from ServiceBase
			void*	GetClientInterface();
			bool	StartRegService();
			bool	StopRegService();


		private:
			CObjectDatabase*		_odb;
			IDatabaseUpdate*		_updateCache;
			GroupManager*			_groupManager;
			NM::Serial::ISerial*	_serialFile;
			bool					_serviceStarted;

		};

// ns

	}
}
