#pragma once
#include "..\..\NetworkModelv2\ObjectDatabase\Interfaces\ObjectDatabaseDefines.h"
#include <set>
#include <tuple>
#include <list>

namespace NM
{
	namespace IPRTREE
	{
		struct compareGuid
		{
			bool operator()(const ::NM::ODB::OBJECTUID& guid1, const ::NM::ODB::OBJECTUID& guid2) const
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

		enum class MatchType { Exact, LPM };
		enum class PrefixType { Host, Network, Broadcast, HostRoute };

		const long IPV4BitLength = 32;
		const long VPNRDBitLength = 64;
		const long RDIPV4BitLength = VPNRDBitLength + IPV4BitLength;

		//typedef GUID OBJECTUID;
		//static const OBJECTUID INVALID_OBJECT_UID = -1;
		static const ::NM::ODB::OBJECTUID RDLEAF_OBJECT_UID = ::NM::ODB::INVALID_OBJECT_UID; //INT_MAX;

		typedef unsigned long long RD;				// RD 64 bit int
		typedef unsigned short	RDTYPE;				// 16bit 2B
		typedef unsigned short	RDASN;				// 16bit 2B
		typedef unsigned long	RDVALUE;			// 32bit 4B
		typedef unsigned long	IPV4RAW;			// 32bit 4B
		typedef in_addr			IPV4ADDR;			// in_addr struct
		typedef unsigned __int8	IPV4PREFIXLENGTH;	// 8bit  1B
		//typedef ::std::set<::NM::ODB::OBJECTUID, ::NM::IPRTREE::compareGuid> INTFLIST;
		typedef ::std::set<::NM::ODB::OBJECTUID, compareGuid> INTFLIST;
	
		typedef ::std::list<RD> RDLIST;
		typedef ::std::tuple<IPV4ADDR, IPV4PREFIXLENGTH, INTFLIST> IPV4PREFIX;
		typedef ::std::list<IPV4PREFIX> RDPREFIXLIST;


	}
}

// IP conversion macros
#define IPV4TOLONG(byte1,byte2,byte3,byte4)  ( (IPV4RAW) (	((IPV4RAW)(b1)<<24)	+	((IPV4RAW)(b2)<<16)	+	((IPV4RAW)(b3)<<8)	+	((IPV4RAW)(b4))	)	)

// Get individual ipv4 octet value
#define IPV4LONGTOBYTE1(ipv4long)	(((ipv4long) >> 24) & 0xff)
#define IPV4LONGTOBYTE2(ipv4long)	(((ipv4long) >> 16) & 0xff)
#define IPV4LONGTOBYTE3(ipv4long)	(((ipv4long) >> 8) & 0xff)
#define IPV4LONGTOBYTE4(ipv4long)	((ipv4long) & 0xff)

// Route Distinguisher conversion macros
#define RDLONGTOTYPE(rd)				( (::NM::IPRTREE::RDTYPE)      (   ((::NM::IPRTREE::RD)(rd) >> 48) & 0xffff					) )
// type 0
#define RDPAIRTOLONGT0(asn,value)		( (::NM::IPRTREE::RD)          (   ((::NM::IPRTREE::RD)(0) << 48)    +    (((::NM::IPRTREE::RD)((asn) & 0xffff)) << 32)    +     ((::NM::IPRTREE::RDVALUE)((value) & 0xffffffff))   ) )
#define RDLONGTOASNT0(rd)				( (::NM::IPRTREE::RDASN)       (   ((::NM::IPRTREE::RD)(rd) >> 32) & 0xffff					) )
#define RDLONGTOVALUET0(rd)				( (::NM::IPRTREE::RDVALUE)     (   ((::NM::IPRTREE::RD)(rd) & 0xffffffff)					) )
// type 1
#define RDPAIRTOLONGT1(ip,value)		( (::NM::IPRTREE::RD)          (   ((::NM::IPRTREE::RD)(0) << 48)    +    (((::NM::IPRTREE::RD)(ip) & 0xffffffff) << 16)    +     ((::NM::IPRTREE::RDVALUE)(value) & 0xffff)   ) )
#define RDLONGTOASNT1(rd)				( (::NM::IPRTREE::RDASN)       (   ((::NM::IPRTREE::RD)(rd) >> 32) & 0xffff					) )
#define RDLONGTOVALUET1(rd)				( (::NM::IPRTREE::RDVALUE)     (   ((::NM::IPRTREE::RD)(rd) & 0xffffffff)					) )
// type 2
#define RDPAIRTOLONGT2(asn,value)		( (::NM::IPRTREE::RD)          (   ((::NM::IPRTREE::RD)(0) << 48)    +    (((::NM::IPRTREE::RD)(asn) & 0xffffffff) << 16)    +     ((::NM::IPRTREE::RDVALUE)(value) & 0xffff)   ) )
#define RDLONGTOASNT2(rd)				( (::NM::IPRTREE::RDASN)       (   ((::NM::IPRTREE::RD)(rd) >> 32) & 0xffff					) )
#define RDLONGTOVALUET2(rd)				( (::NM::IPRTREE::RDVALUE)     (   ((::NM::IPRTREE::RD)(rd) & 0xffffffff)					) )

