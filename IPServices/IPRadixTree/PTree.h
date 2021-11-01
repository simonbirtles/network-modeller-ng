#pragma once
#include "IPRadixDef.h"
#include "..\..\NetworkModelv2\ObjectDatabase\Interfaces\ObjectDatabaseDefines.h"
#include "..\..\NetworkModelv2\ObjectDatabase\Interfaces\DatabaseObserver.h"
#include "..\..\NetworkModelv2\ObjectDatabase\Interfaces\IDatabaseUpdate.h"
#include <list>
#include <set>
#include <tuple>
#include <memory>


namespace NM
{
	namespace ODB
	{
		class IObjectDatabase;
		class ITrigger;
	}

	

	namespace IPRTREE
	{
		
		class PTNode;

		class PTree : public ::NM::ODB::CDatabaseObserver
		{
		public:
			PTree();
			~PTree();

			INTFLIST		Find(RDASN RDAsn, RDVALUE RDValue, IPV4ADDR& ipv4Address, IPV4PREFIXLENGTH prefixLength, MatchType matchType, IPV4ADDR& networkPrefix, IPV4PREFIXLENGTH& networkPrefixLength, bool& found);
			bool			IsRouteDistinguisher(RDASN RDAsn, RDVALUE RDValue);
			RDLIST			EnumRouteDistinguishers();
			RDPREFIXLIST	EnumRouteDistinguisherIPPrefixes(RD);
			bool			GetAggregateIPv4(RDASN RDAsn, RDVALUE RDValue, IPV4ADDR& ipv4, IPV4PREFIXLENGTH prefixLength, bool ignoreMissingPrefix, IPV4PREFIX&);
			PrefixType		GetPrefixType(IPV4ADDR& ipv4, IPV4PREFIXLENGTH prefixLength);
			void			GetNetworkAddress(IPV4ADDR& ipv4, IPV4PREFIXLENGTH prefixLength, IPV4ADDR& networkAddress);
			void			GetBroadcastAddress(IPV4ADDR& ipv4, IPV4PREFIXLENGTH prefixLength, IPV4ADDR& broadcastAddress);
			void			GetDottedDecimalMask(IPV4PREFIXLENGTH prefixLength, IPV4ADDR& dottedDecimalMask);

			// Database UpdateCache Calls
			void DatabaseUpdate();
			// database triggers
			bool InterfaceDeleteTrigger(::NM::ODB::IObjectDatabase&, ::NM::ODB::OBJECTUID, const ::NM::ODB::Value& pValue, const ::NM::ODB::Value& nValue);
			bool InterfaceInsertTrigger(::NM::ODB::IObjectDatabase&, ::NM::ODB::OBJECTUID, const ::NM::ODB::Value& pValue, const ::NM::ODB::Value& nValue);
			bool IPv4AddressUpdateTrigger(::NM::ODB::IObjectDatabase&, ::NM::ODB::OBJECTUID, const ::NM::ODB::Value& pValue, const ::NM::ODB::Value& nValue);
			bool IPv4PrefixLengthUpdateTrigger(::NM::ODB::IObjectDatabase&, ::NM::ODB::OBJECTUID, const ::NM::ODB::Value& pValue, const ::NM::ODB::Value& nValue);

		private:
			PTNode* _root;
			::NM::ODB::IDatabaseUpdate::UpdateQueueHandle _pUpdateQueue;
			::std::unique_ptr<::NM::ODB::IObjectDatabase> _odb;
			::std::shared_ptr<::NM::ODB::ITrigger> _triggerManager;
			::NM::ODB::TRIGGERID			_deleteTriggerID;
			::NM::ODB::TRIGGERID			_insertTriggerID;
			::NM::ODB::TRIGGERID			_updateIPv4AddressTriggerID;
			::NM::ODB::TRIGGERID			_updateIPv4PrefixLengthTriggerID;

			// these methods by nature are type 0 (RDASN[16bit] & RDVALUE[32bit]), poss change or add other type methods or generic Longlong methods
			void			Insert(RDASN RDAsn, RDVALUE RDValue, IPV4ADDR& ipv4, IPV4PREFIXLENGTH prefixLength, ::NM::ODB::OBJECTUID intfUID);
			void			Delete(RD rd, IPV4ADDR& ipv4, IPV4PREFIXLENGTH prefixLength, bool bRecursive);
			void			DeleteRouteDistinguisher(RDASN RDAsn, RDVALUE RDValue);

			void	AddChild(PTNode* pParentNode, PTNode* pChildNode, bool Right);
			PTNode* InsertRouteDistinguisher(RD rd);
			PTNode* FindRouteDistinguisher(RD rd);
			PTNode* InsertIPv4Address(PTNode*, IPV4ADDR& ipv4Address, IPV4PREFIXLENGTH prefixLength, PrefixType prefixType);
			PTNode* InsertAddressNode(PTNode* pParentLeaf, IPV4ADDR& ipv4Address, IPV4PREFIXLENGTH prefixLength, PrefixType prefixType);
			PTNode* FindIPv4Address(PTNode*, IPV4ADDR& ipv4Address, IPV4PREFIXLENGTH prefixLength, MatchType, IPV4ADDR& networkPrefix, IPV4PREFIXLENGTH& networkPrefixLength);
			void	FindLastBranch(PTNode* startNode, IPV4ADDR& ipv4Address, IPV4PREFIXLENGTH prefixLength, PTNode** ppParentNode, PTNode** ppChildNode);
			void	DeleteRecursive(PTNode* parentNode);
			void	EnumRouteDistinguishers(RDLIST& rdList, RD rd, PTNode* nextNode);
			void	EnumRouteDistinguisherIPPrefixes(RDPREFIXLIST& prefixList, IPV4RAW ipAddr, IPV4PREFIXLENGTH prefixLength, PTNode* nextNode);
			PTNode* GetIPv4PrefixAggregate(PTNode* pRDLeafNode, IPV4ADDR& ipv4, IPV4PREFIXLENGTH prefixLength, bool ignoreMissingPrefix, IPV4PREFIX& aggregate);
			PTNode* FindNextEntry(PTNode* parentNode);

			void	InsertInterface(PTNode*, ::NM::ODB::OBJECTUID);
			size_t  DeleteInterface(RD rd, IPV4ADDR& ipv4Address, IPV4PREFIXLENGTH prefixLength, ::NM::ODB::OBJECTUID);
			bool	HasInterface(PTNode*, ::NM::ODB::OBJECTUID);
			void	GetInterfaces(PTNode*, INTFLIST&);
			size_t	GetInterfaceCount(PTNode*);
			void	DeleteNode(PTNode*);
			void	ClearRadixTreeData();

			void	RefreshRadixTreeData();
			void	InsertPrefix(::std::shared_ptr<::NM::ODB::DatabaseUpdateRecord>);
			void	UpdatePrefix(::std::shared_ptr<::NM::ODB::DatabaseUpdateRecord>);
			void	DeletePrefix(::std::shared_ptr<::NM::ODB::DatabaseUpdateRecord>);




		};


	}
}

