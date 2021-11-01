#include "stdafx.h"
#include "Key.h"

namespace NM
{
	namespace ODB
	{
		size_t Key::_refCnt = 0;

		Key::Key(::std::function<void()> func):
				_f(func)
			{
				// Lock Updates
				incRefCnt();				// increment reference count
			}

			Key::~Key()
			{
				decRefCnt();				// decrement reference count
				if( _refCnt == 0 )			
				{
					_f();					// call function passed when Key unlocks
				}
		
			}

			size_t Key::GetRefCnt()
			{
				return _refCnt;
			}
		
			void Key::decRefCnt()
			{
				_refCnt--;
			}
		
			void Key::incRefCnt()
			{
				_refCnt++;
			}
// ns
	}
}

