#pragma once
#include "TypeDefs.h"
#include <vector>
#include <map>
#include <assert.h>

namespace NM 
{
	namespace ODB 
	{
		class CType
		{
		public:
			explicit CType(TypeT typeId):
				_ctypeId(typeId)
			{
			}
			/**
			* NewValue
			*
			* Returns a copy (clone) of the stored default value for the attribute.
			*/
			BaseValue* NewValue() const
			{
				//assert(false);
				//todo change to .find() and return nullptr if not valid Type - get rid of unknown ???
				return _prototypes[_ctypeId]->Clone();
			}

			TypeT GetType() const
			{
				return _ctypeId;
			}
	
			// create templates for each CType to clone for newvalue()
			static void InitDataTypes()
			{
				_prototypes[TypeT::TypeString]		= new real_string(L"");					// RealValue<ODBWString>(L"");
				_prototypes[TypeT::TypeInt]			= new real_int(0);						// RealValue<ODBInt>(0);
				_prototypes[TypeT::TypeDouble]		= new real_double(0);					// RealValue<ODBDouble>(0);
				_prototypes[TypeT::TypeShort]		= new real_short(0);					// RealValue<ODBShort>(0);
				_prototypes[TypeT::TypeBool]		= new real_bool(false);					// RealValue<ODBBool>(false);
				_prototypes[TypeT::TypeLongLong]	= new real_longlong(0);					// RealValue<ODBLongLong>(false);
				_prototypes[TypeT::TypeIPv4]		= new real_inaddr;						// RealValue<ODBINADDR>(inaddr);
				_prototypes[TypeT::TypeByte]		= new real_byte(0);						// RealValue<ODBByte>(0);
				_prototypes[TypeT::TypeColor]		= new real_color;						// 0x00ff000 = Red

				tm datetime;
				::std::memset(&datetime, 0, sizeof(datetime));
				datetime.tm_mday = 1;
				datetime.tm_isdst = -1;
				datetime.tm_year = 100;		// year 2000
				_prototypes[TypeT::TypeTime]		= new real_time(datetime);				//RealValue<ODBTime>(datetime);

				GUID uid;
				::std::memset(&uid, 0, sizeof(uid));
				_prototypes[TypeT::TypeUID]			= new real_uid(uid);					//RealValue<ODBUID>(uid);

				_prototypes[TypeT::TypeUnknown]		= new real_string(L"");					//RealValue<ODBWString>(L"");

				ODBVectorUID emptyUIDVec;
				_prototypes[TypeT::TypeVector]		= new real_vector(emptyUIDVec);					//RealValue<ODBWString>(L"");

			}

			static void CleanUpDataTypes()
			{
				std::map<TypeT, BaseValue*>::iterator it = _prototypes.begin();
				while( it != _prototypes.end() )
				{
					delete it->second;
					++it;
				}
				_prototypes.clear();

			}

		private:
			TypeT _ctypeId;
			static std::map<TypeT, BaseValue*> _prototypes;
		};
	}
}
