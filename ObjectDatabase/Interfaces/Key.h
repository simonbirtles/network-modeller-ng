#pragma once
#include <functional>

namespace NM
{
	namespace ODB
	{
		/*************************************************************************************************************
		* DatabaseUpdateCache::Key 
		*
		* Locks from client updates, upon release (dtor) will call decrement ref count and call update clients.
		**************************************************************************************************************/
		class Key
		{
		public:
			Key(::std::function<void()> func);
			~Key();
			static size_t GetRefCnt();
		
		private:
			static size_t _refCnt;
			::std::function<void()> _f;		
			void decRefCnt();		
			void incRefCnt();
		};

		// ns
	}
}