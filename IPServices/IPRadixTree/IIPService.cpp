/*
* IIPService
* 
* Client interface to the IP service.. 
*/
#include "stdafx.h"
#include "IIPService.h"
#include "PTree.h"

namespace NM
{
	namespace IPRTREE
	{
		/**
		*
		*
		*/
		IIPService::IIPService(::std::shared_ptr<PTree> pPTNode):
			_pIPService(pPTNode)
		{
		}
		/**
		*
		*
		*/
		IIPService::~IIPService()
		{
		}
		/**
		* Find
		*
		* Find the leaf for the given ipaddr and prefix, based on matchtype enum
		* Returns the objectuid (interface) associated with the address
		*       networkPrefix is the matching network - either Exact or LPM
		*		networkPrefixLength is the matching network mask length - either exact or LPM
		*		found boolean if search was successful
		*/
		INTFLIST IIPService::Find(RDASN RDAsn, RDVALUE RDValue, IPV4ADDR& ipv4Address, IPV4PREFIXLENGTH prefixLength, MatchType matchType, IPV4ADDR& networkPrefix, IPV4PREFIXLENGTH& networkPrefixLength, bool& found)
		{
			return _pIPService->Find(RDAsn, RDValue, ipv4Address, prefixLength, matchType, networkPrefix, networkPrefixLength, found);
		}
		/**
		* IsRouteDistinguisher
		*
		* Returns true if the RD exists in the database
		*/
		bool IIPService::IsRouteDistinguisher(RDASN RDAsn, RDVALUE RDValue)
		{
			return _pIPService->IsRouteDistinguisher(RDAsn, RDValue);
		}
		/**
		* EnumRouteDistinguishers
		*
		* Returns a list of Route Distinguishers in the database
		*/
		RDLIST IIPService::EnumRouteDistinguishers()
		{
			return _pIPService->EnumRouteDistinguishers();
		}
		/**
		* EnumRouteDistinguisherIPPrefixes
		*
		* Returns a list of IP Prefixes contained in the given RD
		*/
		RDPREFIXLIST IIPService::EnumRouteDistinguisherIPPrefixes(RD rd)
		{
			return _pIPService->EnumRouteDistinguisherIPPrefixes(rd);
		}
		/**
		*
		*
		*/
		bool IIPService::GetAggregateIPv4(RDASN RDAsn, RDVALUE RDValue, IPV4ADDR& ipv4, IPV4PREFIXLENGTH prefixLength, bool ignoreMissingPrefix, IPV4PREFIX& ipv4Prefix)
		{
			return _pIPService->GetAggregateIPv4(RDAsn, RDValue, ipv4, prefixLength, ignoreMissingPrefix, ipv4Prefix);
		}
		/**
		* GetPrefixType
		*
		* Helper function, prefix does not have to exist in IP Service, evaluates any given ipv4 prefix
		*/
		PrefixType IIPService::GetPrefixType(IPV4ADDR& ipv4, IPV4PREFIXLENGTH prefixLength)
		{
			return _pIPService->GetPrefixType(ipv4, prefixLength);
		}
		/**
		* GetNetworkAddress
		*
		* Helper function, prefix does not have to exist in IP Service, evaluates any given ipv4 prefix
		*/
		void IIPService::GetNetworkAddress(IPV4ADDR& ipv4, IPV4PREFIXLENGTH prefixLength, IPV4ADDR& networkAddress)
		{
			_pIPService->GetNetworkAddress(ipv4, prefixLength, networkAddress);
		}
		/**
		* GetBroadcastAddress
		*
		* Helper function, prefix does not have to exist in IP Service, evaluates any given ipv4 prefix
		*/
		void IIPService::GetBroadcastAddress(IPV4ADDR& ipv4, IPV4PREFIXLENGTH prefixLength, IPV4ADDR& broadcastAddress)
		{
			_pIPService->GetBroadcastAddress(ipv4, prefixLength, broadcastAddress);
		}
		/**
		* GetDottedDecimalMask
		*
		* Helper function, prefix does not have to exist in IP Service, evaluates any given ipv4 prefix
		*/
		void IIPService::GetDottedDecimalMask(IPV4PREFIXLENGTH prefixLength, IPV4ADDR& dottedDecimalMask)
		{
			_pIPService->GetDottedDecimalMask(prefixLength, dottedDecimalMask);
		}


	}
}
