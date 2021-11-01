#pragma once
#include "..\DatabaseCoreElements\TypeDefs.h"
#include "..\DatabaseCoreElements\AttributeDefines.h"
#include "Key.h"

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <Rpc.h>


namespace NM 
{
	namespace ODB 
	{
		class Value;
		class IAttribute;
		
		typedef ::std::shared_ptr<Key>	LockHandle;

		typedef ODBUID OBJECTUID; 
		typedef ODBUID TRIGGERID;
		static const OBJECTUID INVALID_OBJECT_UID = GUID_NULL; 
		static const size_t ATTRIB_NAME_ELEMENT = 0;
		static const size_t ATTRIB_VALUE_ELEMENT = 1;
		static const int	INVALID_ATTRIBUTE = -1;

		// Object type deeply statically assigned to every created object
		enum class ObjectType		{Vertex = 1, Edge, Flow, Demand, Path, Group, Layer, Database, Interface, ObjectInvalid};
		// Used when getting the first or last object for a VertexType
		enum class SiblingPosition  {First, Last};	
		// DatabaseUpdateType
		enum class DatabaseUpdateType {Create = 1, Update = 2, Delete = 4, Refresh = 8};	

		typedef ::std::list<::std::wstring> ATTRIBUTELIST;
		typedef ::std::shared_ptr<Value> SPVALUE;
		typedef ::std::unique_ptr<Value> UPVALUE;
		typedef ::std::unique_ptr<IAttribute> UPIATTRIBUTE;

		// Attribute/String typedef types & maps for object creation
		typedef ::std::wstring AttributeName;
		typedef ::std::wstring ATTRIBUTENAME;
		typedef ::std::wstring ATTRIBUTESTRINGVALUE;
		typedef ::std::map<ATTRIBUTENAME, ATTRIBUTESTRINGVALUE> OBJECTATTRIBUTES;

		// For XML Document
		typedef ::std::tuple<::std::wstring, ::std::wstring> ATTR_VAL_PAIR;
		typedef ::std::vector<ATTR_VAL_PAIR> ATTRIBUTES;
		typedef ::std::vector<ATTRIBUTES> NODES;

		// for DB triggers
		class IObjectDatabase;
		typedef ::std::function<bool(IObjectDatabase&, OBJECTUID, const Value& pValue, const Value& nValue)> TriggerFunc;
		enum class TriggerOperation { Insert, Update, Delete };


		struct compareGuid
		{
			bool operator()(const ODBUID& guid1, const ODBUID& guid2) const
			{
				if (guid1.Data1 != guid2.Data1) {
					return guid1.Data1 < guid2.Data1;
				}
				if (guid1.Data2 != guid2.Data2) {
					return guid1.Data2 < guid2.Data2;
				}
				if (guid1.Data3 != guid2.Data3) {
					return guid1.Data3 < guid2.Data3;
				}
				for (int i = 0;i<8;i++) {
					if (guid1.Data4[i] != guid2.Data4[i]) {
						return guid1.Data4[i] < guid2.Data4[i];
					}
				}
				return false;
			}
		};


// namespace end
	}
}



		
		/// IP Protocol Type
		//enum class IPProtocol		{TCP, UDP, ICMP, None};
		/// Vertex Type
		//enum class VertexType		{Router = 1, Switch = 2 };
		/// Edge Type
		//enum class EdgeType			{Backplane = 1, DWDM, Ethernet, SONET, SDH, ATM, FrameRelay, T1, E1, T3, E3, ISDN, SMDS, ADSL, ADSL2, ADSL2plus, VDSL, VDSL2 };
		///
		//enum class AttributeFilter	{All, UserModify, Required, Serialize, Locked };
		/// Borders for drawn objects
		//enum class BorderLineType	{Solid = 1, Dash = 2, Dot = 3, DashDot = 4};		 
		/// Measurement Units for Time Intervals
		//enum class TimeUnit			{Year = 1, Month, Day, Hour, Minute, Second, MilliSecond, MicroSecond, NanoSecond, PicoSecond} ;
		/// Measurement Units for Links
		//enum class BandwidthUnits   {bps = 1, Kbps, Mbps, Gbps, Tbps}; 
