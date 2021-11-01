/*
* IIPService
*
* Client interface to the IP service..
*/
#pragma once
#include "IPRadixDef.h"

#include <memory>

namespace NM
{
	namespace IPRTREE
	{
		class PTree;

		class IIPService
		{
		public:
			IIPService(::std::shared_ptr<PTree>);
			~IIPService();

			// these methods by nature are type 0 (RDASN[16bit] & RDVALUE[32bit]), poss change or add other type methods or generic Longlong methods
			INTFLIST		Find(RDASN RDAsn, RDVALUE RDValue, IPV4ADDR& ipv4Address, IPV4PREFIXLENGTH prefixLength, MatchType matchType, IPV4ADDR& networkPrefix, IPV4PREFIXLENGTH& networkPrefixLength, bool& found);
			bool			IsRouteDistinguisher(RDASN RDAsn, RDVALUE RDValue);
			RDLIST			EnumRouteDistinguishers();
			RDPREFIXLIST	EnumRouteDistinguisherIPPrefixes(RD);
			bool			GetAggregateIPv4(RDASN RDAsn, RDVALUE RDValue, IPV4ADDR& ipv4, IPV4PREFIXLENGTH prefixLength, bool ignoreMissingPrefix, IPV4PREFIX&);
			PrefixType		GetPrefixType(IPV4ADDR& ipv4, IPV4PREFIXLENGTH prefixLength);
			void			GetNetworkAddress(IPV4ADDR& ipv4, IPV4PREFIXLENGTH prefixLength, IPV4ADDR& networkAddress);
			void			GetBroadcastAddress(IPV4ADDR& ipv4, IPV4PREFIXLENGTH prefixLength, IPV4ADDR& broadcastAddress);
			void			GetDottedDecimalMask(IPV4PREFIXLENGTH prefixLength, IPV4ADDR& dottedDecimalMask);
			

		private:
			::std::shared_ptr<PTree>	_pIPService;

		};

	}
}

