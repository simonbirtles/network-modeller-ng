#pragma once

namespace NM 
{
	namespace ODB 
	{
		class CDatabaseObserver
		{
		public:
			CDatabaseObserver(void){};
			virtual ~CDatabaseObserver(void){};

			virtual void DatabaseUpdate() = 0;

		};
	}
}

