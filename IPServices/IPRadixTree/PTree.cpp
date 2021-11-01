/*
* PTree.cpp
*
* Manages the radix (binary) tree and provides public functions for insert, delete, find and enumeration.
*
* Tree manages: (including related Route Distinguisher for each entry)
*	1. Subnets/Aggregates with no assigned interface(s)
*	2. Subnets/Aggregates with assigned interface(s) as next hop for routing outside of the netmodel autonomous system, this would be the boundary AS point of advertising this netwok prefix into the AS
*	3. Subnet hosts in form (i.e. 192.168.10.3/24) with assigned interface
*	4. Host routes in for (x.x.x.x/32) with assigned interface for either outbound routing as point 2. or for interface assignment
*
*/
#include "stdafx.h"
#include "PTree.h"
#include "PTNode.h"
#include "..\..\ServiceRegistry\IServiceRegistry.h"
#include "IObjectDatabase.h"
#include "ITrigger.h"
#include <string>
#include <assert.h>

extern NM::Registry::IServiceRegistry* reg;

namespace NM
{
	namespace IPRTREE
	{


		/**
		* CTOR
		*
		*/
		PTree::PTree()
		{
			_root = new PTNode();

			::std::unique_ptr<::NM::ODB::IDatabaseUpdate> _updateCache(static_cast<::NM::ODB::IDatabaseUpdate*>(reg->GetClientInterface(L"OBJECTUPDATECACHE")));

			::std::vector<::std::wstring> attributeList;
			attributeList.push_back(L"create");
			attributeList.push_back(L"delete");
			attributeList.push_back(L"rd");
			attributeList.push_back(L"ipaddress");
			attributeList.push_back(L"ipprefixlength");
			_pUpdateQueue = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Interface, attributeList);
			attributeList.clear();
			attributeList.push_back(L"*");
			_pUpdateQueue = _updateCache->RequestClientUpdatesQueue(this, ::NM::ODB::ObjectType::Database, attributeList);  //DatabaseUpdateType::Refresh

			_odb.reset(static_cast<::NM::ODB::IObjectDatabase*>(reg->GetClientInterface(L"ODB")));

			::std::shared_ptr<::NM::ODB::ITrigger> _triggerManager;
			_triggerManager = _odb->GetTriggerInterface();

			// create triggers...
			// broadcast address deny, only allow host, hostroute & network addresses to be added to the intfacetable
			// create trigger for group deletes
			//_triggerManager = _odb->GetTriggerManagerPtr(); // get service interface

			::std::wstring tableName = L"interfacetable";
			::std::wstring attributeName = L"";
			::NM::ODB::TriggerFunc deletetrigger = std::bind(
				&PTree::InterfaceDeleteTrigger,
				this,
				std::placeholders::_1,
				std::placeholders::_2,
				std::placeholders::_3,
				std::placeholders::_4
				);
			_deleteTriggerID = _triggerManager->CreateTrigger(tableName, attributeName, ::NM::ODB::TriggerOperation::Delete, deletetrigger);

			::NM::ODB::TriggerFunc inserttrigger = std::bind(
				&PTree::InterfaceInsertTrigger,
				this,
				std::placeholders::_1,
				std::placeholders::_2,
				std::placeholders::_3,
				std::placeholders::_4
				);
			_insertTriggerID = _triggerManager->CreateTrigger(tableName, attributeName, ::NM::ODB::TriggerOperation::Insert, inserttrigger);

			attributeName = L"ipaddress";
			::NM::ODB::TriggerFunc updatetrigger = std::bind(
				&PTree::IPv4AddressUpdateTrigger,
				this,
				std::placeholders::_1,
				std::placeholders::_2,
				std::placeholders::_3,
				std::placeholders::_4
				);
			_updateIPv4AddressTriggerID = _triggerManager->CreateTrigger(tableName, attributeName, ::NM::ODB::TriggerOperation::Update, updatetrigger);

			attributeName = L"ipprefixlength";
			updatetrigger = std::bind(
				&PTree::IPv4PrefixLengthUpdateTrigger,
				this,
				std::placeholders::_1,
				std::placeholders::_2,
				std::placeholders::_3,
				std::placeholders::_4
				);
			_updateIPv4PrefixLengthTriggerID = _triggerManager->CreateTrigger(tableName, attributeName, ::NM::ODB::TriggerOperation::Update, updatetrigger);

		}
		/**
		* DTOR
		*
		*/
		PTree::~PTree()
		{
			ClearRadixTreeData();
			DeleteNode(_root);

		}
		/**
		* ClearRadixTreeData
		*
		* Removes all data from the radix tree except leaves the root in place
		*/
		void PTree::ClearRadixTreeData()
		{
			DeleteRecursive(_root);
			_root->_left = nullptr;
			_root->_right = nullptr;
			return;
		}
		/**
		* InsertRouteDistinguisher
		* (Private)
		*
		* Inserts a given RD into the tree with the root as the parent.
		*/
		PTNode* PTree::InsertRouteDistinguisher(RD rd)
		{
			//unsigned long long testValue = 0;
			PTNode* pNode = _root;
			PTNode* pTempNode = nullptr;

			// left / right bit testing
			for (long long i = VPNRDBitLength - 1; i >= 0; i--)
			{
				RD rdBit = 1LL << i;

				// bit=1, right side
				if (rd & rdBit)
				{
					// if no child, add one
					if (!pNode->_right)
					{
						PTNode* pChild = new PTNode();
						AddChild(pNode, pChild, true);
						//OutputDebugString(L"\nPTNode size = "); 
						//OutputDebugString(::std::to_wstring(sizeof(*pChild)).c_str());
					}
					pNode = pNode->_right;
				}
				// Bit=0, left side 
				else
				{
					// if no child, add one
					if (!pNode->_left)
					{
						PTNode* pChild = new PTNode();
						AddChild(pNode, pChild, false);

					}
					pNode = pNode->_left;
				}
			}
			//InsertInterface(pNode, RDLEAF_OBJECT_UID);
			//INT pNode->_interfaceUID = RDLEAF_OBJECT_UID;
			return pNode;
		}
		/**
		* InsertIPv4Address
		* (Private)
		*
		* Adds a IPv4 Address to the tree at the given node insertion point as a parent, the leaf of the RD
		* If the AddressType is subnet, the code adds the address bit by bit until the prefixLength is met in terms of tree depth,
		* the final entry is always the subnet address even if a host in the subnet is given in the address.
		* If the AddressType is Host, the entire 32 bits are added to the tree
		*/
		PTNode* PTree::InsertIPv4Address(PTNode* RDLeaf, IPV4ADDR& ipv4Address, IPV4PREFIXLENGTH prefixLength, PrefixType prefixType)
		{
			assert(RDLeaf);
			PTNode* pParentNode = RDLeaf;
			PTNode* pChildNode = nullptr;
			unsigned __int8 byteCount = 0;
			///IPV4PREFIXLENGTH addressPrefixLength = prefixLength;

			if (prefixType == PrefixType::Broadcast) return nullptr;
			///if (prefixType == PrefixType::Host) addressPrefixLength = 32;

			//The htonl function converts a u_long from **host** to **TCP/IP network byte order** (which is big-endian). i.e. from 237.0.168.192 -> 192.168.0.237
			unsigned long hladdr = htonl(ipv4Address.S_un.S_addr);

			// do we have aggregate for this node?
			IPV4PREFIX aggregate;
			PTNode* pAggregateAddressLeaf = GetIPv4PrefixAggregate(RDLeaf, ipv4Address, prefixLength, true, aggregate);
			pAggregateAddressLeaf = (pAggregateAddressLeaf == nullptr ? RDLeaf : pAggregateAddressLeaf);

			// get the network address (closest aggregate for the given address)
			IPV4ADDR networkAddress;
			GetNetworkAddress(ipv4Address, prefixLength, networkAddress);

			// if the network address aggregate already exists, insert as child here.
			if (networkAddress.S_un.S_addr == ::std::get<0>(aggregate).S_un.S_addr)
			{
				pChildNode = InsertAddressNode(pAggregateAddressLeaf, ipv4Address, prefixLength, prefixType);
			}
			else
			{
				PTNode* pNetworkLeaf = pAggregateAddressLeaf;
				if (networkAddress.S_un.S_addr != ipv4Address.S_un.S_addr)
				{
					// otherwise create the network address node, 
					pNetworkLeaf = InsertAddressNode(pAggregateAddressLeaf, networkAddress, prefixLength, PrefixType::Network);
				}
				// then add the given prefix to that node as child
				pChildNode = InsertAddressNode(pNetworkLeaf, ipv4Address, prefixLength, prefixType);

			}
			return pChildNode;
		}
		/**
		*
		*
		*
		*
		*/
		PTNode* PTree::InsertAddressNode(PTNode* pParentLeaf, IPV4ADDR& ipv4Address, IPV4PREFIXLENGTH prefixLength, PrefixType prefixType)
		{
			IPV4PREFIXLENGTH parentPrefixLenth = pParentLeaf->_prefixLength;
			assert(prefixLength >= parentPrefixLenth);
			assert(prefixLength <= IPV4BitLength);
			PTNode* pChildNode = pParentLeaf;
			PTNode* pParentNode = pParentLeaf;
			IPV4PREFIXLENGTH addressPrefixLength = prefixLength;

			// if the given prefix is a host in a subnet (not host route), 
			// change loop to add all 32 bits, not just to mask end.
			if (prefixType == PrefixType::Host) addressPrefixLength = 32;

			//The htonl function converts a u_long from **host** to **TCP/IP network byte order** (which is big-endian). i.e. from 237.0.168.192 -> 192.168.0.237
			unsigned long hladdr = htonl(ipv4Address.S_un.S_addr);

			// insert the given address, add intermediate nodes where required, starting at the given parent
			for (long i = 32 - 1 - parentPrefixLenth; i >= 32 - addressPrefixLength; i--)
			{
				// get decimal bit value for this bit
				unsigned long ipv4bitsb = 1L << i;

				// if this bit was set, get the 8bit word decimal value of the bit 
				// bit=1, right side
				pChildNode = (hladdr & ipv4bitsb) ? pParentNode->_right : pParentNode->_left;

				// if no child, add one for this bit
				if (!pChildNode)
				{
					PTNode* pChild = new PTNode();
					AddChild(pParentNode, pChild, ((hladdr & ipv4bitsb) ? true : false));
					pChildNode = pChild;
				}
				// set next node to child node - move down the tree one level (node)
				pParentNode = pChildNode;
			}

			// if this is a host in a subnet i.e. 192.168.12.56/24 or its a host route x.x.x.x/32 
			// set the prefixlength for the host address on the 32nd bit (last bit in the address)
			//if ((prefixType == PrefixType::Host) || (prefixLength == 32))
			pChildNode->_prefixLength = prefixLength;

			return pChildNode;
		}
		/**
		* AddChild
		* (Private)
		*
		* Adds a given child PTNode to a given parent PTNode
		*/
		void PTree::AddChild(PTNode* pParentNode, PTNode* pChildNode, bool Right)
		{
			// left or right ?
			PTNode** pParentCurrentChildNode = (Right ? &pParentNode->_right : &pParentNode->_left);
			// child node pointer must be nullptr to add child
			assert(*pParentCurrentChildNode == nullptr);

			// set child either left or right 
			*pParentCurrentChildNode = pChildNode;
			// set bit index on child to be 1 greater then ours
			pChildNode->_bitIndex = pParentNode->_bitIndex + 1;

			pParentCurrentChildNode = nullptr;
			return;
		}
		/**
		* FindRouteDistinguisher
		* (Private)
		*
		* Searches for a given 64Bit RD and returns the leaf node pointer
		*/
		PTNode* PTree::FindRouteDistinguisher(RD rd)
		{
			PTNode* pNode = _root;
			PTNode* pNodeTemp = nullptr;

			// left / right bit testing
			for (int i = VPNRDBitLength - 1; i >= 0; i--)
			{
				RD rdBit = 1LL << i;
				pNodeTemp = (rd & rdBit) ? pNode->_right : pNode->_left;			// bit=1, right side
				if (!pNodeTemp)
					return nullptr;
				pNode = pNodeTemp;
			}
			return pNode;
		}
		/**
		* FindIPv4Address
		* (Private)
		*
		* Searches for a given RD(64Bit) and IPv4 Address and returns the leaf node pointer and the matching network prefix based on matchtype (LPM/Exact)
		*/
		PTNode* PTree::FindIPv4Address(PTNode* RDLeaf, IPV4ADDR& ipv4Address, IPV4PREFIXLENGTH prefixLength, MatchType matchType, IPV4ADDR& networkPrefix, IPV4PREFIXLENGTH& networkPrefixLength)
		{
			assert(RDLeaf);
			PTNode* pNode = RDLeaf;
			PTNode* pLastLPMNode = nullptr;
			PTNode* pChildNode = nullptr;
			///unsigned __int8 byteCount = 0;
			unsigned __int8 ipbytes[4];
			unsigned __int8 ipLastLPMbytes[4];
			unsigned __int8* pByte = &ipbytes[0];
			::std::memset(ipbytes, 0, sizeof(unsigned __int8) * 4);
			::std::memset(ipLastLPMbytes, 0, sizeof(unsigned __int8) * 4);
			::std::memset(&networkPrefix, 0, sizeof(IPV4ADDR));
			unsigned long ipv4bitsb = 0;
			IPV4PREFIXLENGTH networkPrefixLengthCount = 0;
			IPV4PREFIXLENGTH prefixLengthBits = prefixLength;

			// check valid addr/prefix length
			PrefixType prefixType = GetPrefixType(ipv4Address, prefixLength);
			if (prefixType == PrefixType::Broadcast) return nullptr;
			if (prefixType == PrefixType::Host) prefixLengthBits = 32;

			//The htonl function converts a u_long from **host** to **TCP/IP network byte order** (which is big-endian). i.e. from 237.0.168.192 -> 192.168.0.237
			unsigned long hladdr = htonl(ipv4Address.S_un.S_addr);

			// left(0) / right(1) bit testing
			for (long i = 32 - 1; i >= (32 - prefixLengthBits); i--)
			{
				++networkPrefixLengthCount;
				// get decimal value for this bit
				ipv4bitsb = 1L << i;

				// if this bit was set, set temp pointer to left or right child of the current node 
				// bit=1=right side, 
				if (hladdr & ipv4bitsb)
				{
					*pByte += 1 << (i % 8);
					pChildNode = pNode->_right;
				}
				else
				{
					pChildNode = pNode->_left;
				}

				// check we have valid child node on this side of the current node
				// and if not, terminate the search 
				if (!pChildNode)
					break;

				// if we passed one byte (8bits) move pointer to next octet to sum octet value
				if (i % 8 == 0)
					pByte++;

				// save temp reference to this node if its a LPM 
				//INT if (pChildNode->_interfaceUID != ::NM::ODB::INVALID_OBJECT_UID)
				if (GetInterfaceCount(pChildNode) != 0)
				{
					pLastLPMNode = pChildNode;	// reference to current node
					// save copy of the current network address - the prefix
					::std::memcpy(&ipLastLPMbytes, &ipbytes, sizeof(unsigned __int8) * 4);
					networkPrefixLength = networkPrefixLengthCount;
				}

				// set next node to process
				pNode = pChildNode;
			}

			// return result depending on requested matching type
			switch (matchType)
			{
			case MatchType::Exact:
			{
				//INT if ((!pChildNode) || (pChildNode->_interfaceUID == ::NM::ODB::INVALID_OBJECT_UID))
				if ((!pChildNode) || (GetInterfaceCount(pChildNode) == 0))
				{
					pLastLPMNode = nullptr;	// not found
					networkPrefixLength = 0;
				}
				else
				{
					::std::memcpy(&networkPrefix, &ipv4Address, sizeof(IPV4ADDR));
					networkPrefixLength = prefixLength;
				}
			}
			break;

			case MatchType::LPM:
			{
				// check this node has an assigned interface, otherwise its just a transit node in the tree
				// that represents a partial ip address which is not assigned to any interface
				if (pLastLPMNode)
				{  // found LPM, so update passed struct with actual network found based on the prefix passed
					networkPrefix.S_un.S_un_b.s_b1 = ipLastLPMbytes[0];
					networkPrefix.S_un.S_un_b.s_b2 = ipLastLPMbytes[1];
					networkPrefix.S_un.S_un_b.s_b3 = ipLastLPMbytes[2];
					networkPrefix.S_un.S_un_b.s_b4 = ipLastLPMbytes[3];
				}
				else
				{
					networkPrefixLength = 0;
				}
			}
			break;

			default:
				assert(false);
				break;
			}

			return pLastLPMNode;
		}
		/**
		* FindRouteDistinguisher
		* (Public)
		*
		* this method by nature of RDASN & RDVALUE is a type 0
		* http://packetlife.net/blog/2013/jun/10/route-distinguishers-and-route-targets/
		*/
		bool PTree::IsRouteDistinguisher(RDASN RDAsn, RDVALUE RDValue)
		{
			// Route Distinguisher
			unsigned long long rd = (static_cast<unsigned long long>(RDAsn) << 32) + RDValue;
			PTNode* pRDLeaf = FindRouteDistinguisher(rd);

			in_addr foundNetwork;
			::std::memset(&foundNetwork, 0, sizeof(in_addr));

			return (pRDLeaf != nullptr ? true : false);
		}
		/**
		* Find
		* (Public)
		*
		* Find the leaf for the given ipaddr and prefix, based on matchtype enum
		* Returns the objectuid (interface) associated with the address
		*       networkPrefix is the matching network - either Exact or LPM
		*		networkPrefixLength is the matching network mask length - either exact or LPM
		*/
		INTFLIST PTree::Find(RDASN RDAsn, RDVALUE RDValue, IPV4ADDR& ipv4Address, IPV4PREFIXLENGTH prefixLength, MatchType matchType, IPV4ADDR& networkPrefix, IPV4PREFIXLENGTH& networkPrefixLength, bool& found)
		{
			INTFLIST interfaceList;
			found = false;
			::std::memset(&networkPrefix, 0, sizeof(IPV4ADDR));
			networkPrefixLength = 0;

			// Route Distinguisher
			unsigned long long rd = (static_cast<unsigned long long>(RDAsn) << 32) + RDValue;
			PTNode* pRDLeaf = FindRouteDistinguisher(rd);
			if (!pRDLeaf)
				return INTFLIST{}; 

			// search for given ipv4 prefix
			PTNode* pIPNode = FindIPv4Address(pRDLeaf, ipv4Address, prefixLength, matchType, networkPrefix, networkPrefixLength);
			if (pIPNode)
			{
				found = true;
				GetInterfaces(pIPNode, interfaceList);
			}

			return interfaceList;
		}
		/**
		* Insert
		* (Public)
		* this method by nature of RDASN & RDVALUE is a type 0
		* http://packetlife.net/blog/2013/jun/10/route-distinguishers-and-route-targets/
		*/
		void PTree::Insert(RDASN RDasn, RDVALUE RDValue, IPV4ADDR &ipv4, IPV4PREFIXLENGTH prefixLength, ::NM::ODB::OBJECTUID intfUID)
		{
			// check valid addr/prefix length
			PrefixType prefixType = GetPrefixType(ipv4, prefixLength);
			if (prefixType == PrefixType::Broadcast) return;

			// Route Distinguisher
			unsigned long long rd = (static_cast<unsigned long long>(RDasn) << 32) + RDValue;
			PTNode* pRDLeaf = InsertRouteDistinguisher(rd);
			assert(pRDLeaf);

			// IP Address
			PTNode* pLeafNode = InsertIPv4Address(pRDLeaf, ipv4, prefixLength, prefixType);
			if (pLeafNode)
				InsertInterface(pLeafNode, intfUID);

			return;
		}
		/**
		*
		*
		*
		*/
		PrefixType PTree::GetPrefixType(IPV4ADDR& ipv4, IPV4PREFIXLENGTH prefixLength)
		{
			if (prefixLength == 32) return PrefixType::HostRoute;

			// get network mask
			IPV4ADDR networkMask;
			GetDottedDecimalMask(prefixLength, networkMask);
			//networkMask.S_un.S_addr = htonl((0xFFFFFFFF << (32 - prefixLength - 32)) & 0xFFFFFFFF);

			// get network address
			IPV4ADDR networkAddress;
			GetNetworkAddress(ipv4, prefixLength, networkAddress);
			//networkAddress.S_un.S_addr = (ipv4.S_un.S_addr & networkMask.S_un.S_addr);

			// get network broadcast address
			IPV4ADDR networkBroadcast;
			GetBroadcastAddress(ipv4, prefixLength, networkBroadcast);
			//networkBroadcast.S_un.S_addr = (ipv4.S_un.S_addr | (~networkMask.S_un.S_addr));

			// if the given IP is a network address, its represents a subnet
			if (ipv4.s_addr == networkAddress.s_addr) return PrefixType::Network;
			// if the given IP is a broadcast address
			if (ipv4.s_addr == networkBroadcast.s_addr) return PrefixType::Broadcast;
			// if not a network or broadcast its a host address
			return PrefixType::Host;
		}
		/**
		*
		*
		*
		*/
		void PTree::GetNetworkAddress(IPV4ADDR& ipv4, IPV4PREFIXLENGTH prefixLength, IPV4ADDR& networkAddress)
		{
			// get network mask
			IPV4ADDR networkMask;
			GetDottedDecimalMask(prefixLength, networkMask);
			//networkMask.S_un.S_addr = htonl((0xFFFFFFFF << (32 - prefixLength - 32)) & 0xFFFFFFFF);

			// network address
			networkAddress.S_un.S_addr = (ipv4.S_un.S_addr & networkMask.S_un.S_addr);
			return;
		}
		/**
		*
		*
		*
		*/
		void PTree::GetBroadcastAddress(IPV4ADDR& ipv4, IPV4PREFIXLENGTH prefixLength, IPV4ADDR& broadcastAddress)
		{
			// get network mask
			IPV4ADDR networkMask;
			GetDottedDecimalMask(prefixLength, networkMask);
			//networkMask.S_un.S_addr = htonl((0xFFFFFFFF << (32 - prefixLength - 32)) & 0xFFFFFFFF);

			// network broadcast address
			broadcastAddress.S_un.S_addr = (ipv4.S_un.S_addr | (~networkMask.S_un.S_addr));
			return;
		}
		/**
		* GetDottedDecimalMask
		*
		* Takes a integer prefix length \xx and returns a dotted decimal mask x.x.x.x
		*/
		void PTree::GetDottedDecimalMask(IPV4PREFIXLENGTH prefixLength, IPV4ADDR& dottedDecimalMask)
		{
			dottedDecimalMask.S_un.S_addr = htonl((0xFFFFFFFF << (32 - prefixLength - 32)) & 0xFFFFFFFF);
			return;
		}
		/**
		* Delete
		* (Public)
		* this method by nature of RDASN & RDVALUE is a type 0
		* http://packetlife.net/blog/2013/jun/10/route-distinguishers-and-route-targets/
		*
		* Warning: This function deletes ALL interfaces associated with the IPPrefix AND
		*          if recursive set to TRUE, then also deletes ALL subnetworks/hosts and ALL
		*		   associated interfaces. VERY DESTRUCTIVE - USE WISELY
		*
		* Deletes recursively if bRecursive set true, where this is set false, if there are
		* child prefixes then no delete will occur and the interface uid is set to invalid
		* to preserve the chain down the tree
		*/
		void PTree::Delete(RD rd, IPV4ADDR& ipv4, IPV4PREFIXLENGTH prefixLength, bool bRecursive)
		{
			//unsigned long long rd = (static_cast<unsigned long long>(RDAsn) << 32) + RDValue;

			// recursive set - need to update code to support this forceful delete of all child network enties.
			assert(!bRecursive);

			// if not recursive, i.e dont force delete of child branches if they contain actual network entries
			if (!bRecursive)
			{
				DeleteInterface(rd, ipv4, prefixLength, ::NM::ODB::INVALID_OBJECT_UID); 
			}

			return;
		}
		/**
		*
		*
		*
		*/
		PTNode* PTree::FindNextEntry(PTNode* pParentNode)
		{
			PTNode* pNode = nullptr;
			// start on the left
			if (pParentNode->_left)
			{
				//INT if ((pParentNode->_left->_interfaceUID != ::NM::ODB::INVALID_OBJECT_UID) || (pParentNode->_left->_prefixLength != 0))
				if ((GetInterfaceCount(pParentNode->_left) != 0) || (pParentNode->_left->_prefixLength != 0))
					return pParentNode->_left;

				pNode = FindNextEntry(pParentNode->_left);
				if (pNode) return pNode;
			}

			// now the right
			if (pParentNode->_right)
			{
				// INTif ((pParentNode->_right->_interfaceUID != ::NM::ODB::INVALID_OBJECT_UID) || (pParentNode->_right->_prefixLength != 0))
				if ((GetInterfaceCount(pParentNode->_right) != 0) || (pParentNode->_right->_prefixLength != 0))
					return pParentNode->_right;

				pNode = FindNextEntry(pParentNode->_right);
				if (pNode) return pNode;
			}

			return pNode;
		}
		/**
		* DeleteRouteDistinguisher
		* (Public)
		*
		* Deletes the given Route Distinguisher and all associated ip prefixes for this Route Distinguisher
		*/
		void PTree::DeleteRouteDistinguisher(RDASN RDAsn, RDVALUE RDValue)
		{
			// Get Route Distinguisher Leaf
			unsigned long long rd = (static_cast<unsigned long long>(RDAsn) << 32) + RDValue;
			if (!IsRouteDistinguisher(RDAsn, RDValue))
				return;

			PTNode* pCurrentNode = _root;
			PTNode* pBranchParent = nullptr;
			PTNode* pLastBranchChild = nullptr;
			PTNode* pNodeTemp = nullptr;

			// find last branch before rd leaf.
			for (int i = VPNRDBitLength - 1; i >= 0; i--)
			{
				// left / right bit testing
				RD rdBit = 1LL << i;
				pNodeTemp = (rd & rdBit) ? pCurrentNode->_right : pCurrentNode->_left;			// bit=1, right side
				if (!pNodeTemp)
					return;
				// if the current node branches, get the next node pointer as lastBranchChild 
				if (pCurrentNode->_left && pCurrentNode->_right)
				{
					pBranchParent = pCurrentNode;
					//rdBit = 1LL << (i-1);
					pLastBranchChild = pNodeTemp; //(rd & rdBit) ? pNodeTemp->_right : pNodeTemp->_left;
				}
				pCurrentNode = pNodeTemp;
			}

			// delete 
			DeleteRecursive(pLastBranchChild);

			// set the child pointer of the parent to null, will be one or the other
			if (pBranchParent->_left == pLastBranchChild)
				pBranchParent->_left = nullptr;
			else if (pBranchParent->_right == pLastBranchChild)
				pBranchParent->_right = nullptr;
			else
				throw ::std::runtime_error("PTree::Delete\tsomething gone wrong...");

			DeleteNode(pLastBranchChild);

			return;
		}
		/**
		* DeleteRecursive
		* (Private)
		*
		* delete given nodes children recursively
		*/
		void PTree::DeleteRecursive(PTNode* parentNode)
		{
			// start on the left
			if (parentNode->_left)
			{
				DeleteRecursive(parentNode->_left);
				DeleteNode(parentNode->_left);
				parentNode->_left = nullptr;
			}

			// now the right
			if (parentNode->_right)
			{
				DeleteRecursive(parentNode->_right);
				DeleteNode(parentNode->_right);
				parentNode->_right = nullptr;
			}
			return;
		}
		/**
		* FindLastBranch
		* (Private)
		*
		* Returns the last node which branches left and right for the given ip addr and prefix, also provides the next hop child from that branch left/right
		*/
		void PTree::FindLastBranch(PTNode* startNode, IPV4ADDR& ipv4Address, IPV4PREFIXLENGTH prefixLength, PTNode** ppParentNode, PTNode** ppChildNode)
		{
			assert(startNode);
			PTNode* pNode = startNode;
			///PTNode* pLastNodeBranch = startNode;
			PTNode* pChildNode = nullptr;
			unsigned long ipv4bitsb = 0;
			IPV4PREFIXLENGTH prefixLengthBits = prefixLength;

			//The htonl function converts a u_long from **host** to **TCP/IP network byte order** (which is big-endian). i.e. from 237.0.168.192 -> 192.168.0.237
			unsigned long hladdr = htonl(ipv4Address.S_un.S_addr);
			// set default parent and child nodes at the passed start point, 
			// in case no branches or shorter matches occur for this prefix
			ipv4bitsb = 1L << 31;
			*ppParentNode = startNode;
			*ppChildNode = (hladdr & ipv4bitsb) ? startNode->_right : startNode->_left;

			// check valid addr/prefix length
			PrefixType prefixType = GetPrefixType(ipv4Address, prefixLength);
			if (prefixType == PrefixType::Broadcast) return;
			if (prefixType == PrefixType::Host) prefixLengthBits = 32;

			// left(0) / right(1) bit testing
			for (long i = 32 - 1; i >= (32 - prefixLengthBits) + 1; i--)
			{
				// get decimal value for this bit
				ipv4bitsb = 1L << i;

				// if this bit was set, set temp pointer to left or right child of the current node 
				// bit=1=right side, 
				pChildNode = (hladdr & ipv4bitsb) ? pNode->_right : pNode->_left;

				// check we have valid child node on this side of the current node
				// and if not, terminate the search , ip address passed is not in tree
				if (!pChildNode)
				{
					*ppParentNode = nullptr;
					*ppChildNode = nullptr;
					return;
				}

				// save temp reference to this node if it branches left and right or we have found a prefix assigned to interface
				//INT if ((pChildNode->_left && pChildNode->_right) || (pChildNode->_interfaceUID != ::NM::ODB::INVALID_OBJECT_UID))
				if ((pChildNode->_left && pChildNode->_right) || (GetInterfaceCount(pChildNode) != 0))
				{
					ipv4bitsb = (1L << (i - 1));
					// save parent node, is either the branching node or is assigned to an interface
					*ppParentNode = pChildNode;
					// save the next hop child for this prefix from the parent node
					*ppChildNode = (hladdr & ipv4bitsb) ? pChildNode->_right : pChildNode->_left;
				}

				// set next node to process
				pNode = pChildNode;
			}
			return;
		}
		/**
		* GetRouteDistinguisherList
		* (Public)
		* Returns a list of the currently used Route Distinguishers
		* in a 64Bit longlong std::list
		*/
		RDLIST PTree::EnumRouteDistinguishers()
		{
			RDLIST rdList;
			EnumRouteDistinguishers(rdList, 0, _root);	// 0 as not used here but is in recursion
			return rdList;
		}
		/**
		* EnumRouteDistinguishers
		* (Private)
		* Enumerates through the Route Distinguishers in the tree and returns a list
		* in a 64Bit longlong std::list
		*/
		void PTree::EnumRouteDistinguishers(RDLIST& rdList, RD rd, PTNode* nextNode)
		{
			// start on the left
			if ((nextNode->_left) && (nextNode->_left->_bitIndex != VPNRDBitLength))
				EnumRouteDistinguishers(rdList, rd, nextNode->_left);

			// now the right
			if ((nextNode->_right) && (nextNode->_right->_bitIndex != VPNRDBitLength))
			{
				rd += 1LL << (VPNRDBitLength - 1 - nextNode->_right->_bitIndex);
				EnumRouteDistinguishers(rdList, rd, nextNode->_right);
			}

			if (nextNode->_bitIndex == VPNRDBitLength - 1)
				rdList.push_back(rd);

			return;
		}
		/**
		* GetRouteDistinguisherIPPrefixes
		* (Public)
		*
		* Returns a list of ip prefixes, prefix mask length & interface uid for
		* the given RD.
		*/
		RDPREFIXLIST PTree::EnumRouteDistinguisherIPPrefixes(RD rd)
		{
			RDPREFIXLIST rdPrefixList;
			// get rd leaf node (starting point), if not in tree, return empty list.
			PTNode* rdLeafNode = FindRouteDistinguisher(rd);
			if (!rdLeafNode) return rdPrefixList;

			// enum rd prefixes.
			EnumRouteDistinguisherIPPrefixes(rdPrefixList, 0L, 0, rdLeafNode);

			return rdPrefixList;
		}
		/**
		* EnumRouteDistinguisherIPPrefixes
		* (Private)
		*
		* DFS search on tree starting at given node and
		* fills passed list of tuple<IPV4ADDR, IPV4PREFIXLENGTH, ::NM::ODB::OBJECTUID>
		*/
		void PTree::EnumRouteDistinguisherIPPrefixes(RDPREFIXLIST& prefixList, IPV4RAW ipAddr, IPV4PREFIXLENGTH prefixLength, PTNode* nextNode)
		{
			// end of the prefix or interface match found
			//INT if ((nextNode->_bitIndex == RDIPV4BitLength - 1) || (nextNode->_interfaceUID != ::NM::ODB::INVALID_OBJECT_UID) || (nextNode->_prefixLength != 0))
			if ((nextNode->_bitIndex == RDIPV4BitLength - 1) || (GetInterfaceCount(nextNode) != 0) || (nextNode->_prefixLength != 0))
			{
				IPV4ADDR ipa;
				memset(&ipa, 0, sizeof(IPV4ADDR));
				ipa.S_un.S_addr = ntohl(ipAddr);
				INTFLIST interfaceList;
				GetInterfaces(nextNode, interfaceList);
				//INT prefixList.push_back(::std::make_tuple(ipa, (nextNode->_prefixLength != 0 ? nextNode->_prefixLength : prefixLength), nextNode->_interfaceUID));
				prefixList.push_back(::std::make_tuple(ipa, (nextNode->_prefixLength != 0 ? nextNode->_prefixLength : prefixLength), interfaceList));
			}

			++prefixLength;

			// start on the left, no addition done here as bit set as 0 is not a value, just keep recursion going
			// check to see if we have found a prefix by _interfaceUID set
			if ((nextNode->_left) && (nextNode->_left->_bitIndex != RDIPV4BitLength))
			{
				EnumRouteDistinguisherIPPrefixes(prefixList, ipAddr, prefixLength, nextNode->_left);
			}

			// right side (bit set as 1) >> move to next node on right
			if ((nextNode->_right) && (nextNode->_right->_bitIndex != RDIPV4BitLength))
			{
				ipAddr += 1LL << (RDIPV4BitLength - 1 - nextNode->_right->_bitIndex);
				EnumRouteDistinguisherIPPrefixes(prefixList, ipAddr, prefixLength, nextNode->_right);
			}
			return;
		}
		/**
		* GetAggregate
		* (Public)
		*
		* Gets the closest aggregate to the given RD::IPPrefx/length if one exists otherwise returns 0:0:0.0.0.0/0
		* Return bool: true if one found, false if no aggregate exists
		*/
		bool PTree::GetAggregateIPv4(RDASN RDAsn, RDVALUE RDValue, IPV4ADDR& ipv4, IPV4PREFIXLENGTH prefixLength, bool ignoreMissingPrefix, IPV4PREFIX& aggregate)
		{
			// Get RD Leaf 
			unsigned long long rd = (static_cast<unsigned long long>(RDAsn) << 32) + RDValue;
			PTNode* pRDLeaf = FindRouteDistinguisher(rd);
			if (!pRDLeaf)
				return false;

			return GetIPv4PrefixAggregate(pRDLeaf, ipv4, prefixLength, ignoreMissingPrefix, aggregate) != nullptr;
		}
		/**
		* GetIPv4Prefix
		* (Private)
		*
		* Get the aggregate address based on the given ipv4 prefix/len. ignoreMissingPrefix allows a search for an aggregate even if the given
		* more specific does not exist in the table, set to true, the given prefix does not need to be in the tree to find a aggregate for that prefix.
		* Returns: bool: (true)if aggregate found, if so aggregate tuple will be filled in with aggregate details.
		*/
		PTNode* PTree::GetIPv4PrefixAggregate(PTNode* pRDLeafNode, IPV4ADDR& ipv4, IPV4PREFIXLENGTH prefixLength, bool ignoreMissingPrefix, IPV4PREFIX& aggregate)
		{
			IPV4PREFIXLENGTH prefixLengthDepth = prefixLength;
			unsigned __int8 ipbytes[4];
			unsigned __int8 ipLastLPMbytes[4];
			unsigned __int8* pByte = &ipbytes[0];
			::std::memset(ipbytes, 0, sizeof(unsigned __int8) * 4);
			::std::memset(ipLastLPMbytes, 0, sizeof(unsigned __int8) * 4);
			bool bAggregateFound = false;
			unsigned long ipv4bitsb = 0;
			IPV4PREFIXLENGTH lastLPMPrefixLength = 0;
			IPV4PREFIXLENGTH networkPrefixLengthCount = 0;
			INTFLIST lastLPMInterfaceUID;
			::NM::ODB::OBJECTUID lastInterfaceUID = ::NM::ODB::INVALID_OBJECT_UID;
			PTNode* pLastLPMNode = nullptr;
			PTNode* pChildNode = nullptr;
			PTNode* pNode = pRDLeafNode;  // starting point
			unsigned long hladdr = htonl(ipv4.S_un.S_addr);  //The htonl function converts a u_long from **host** to **TCP/IP network byte order** (which is big-endian). i.e. from 237.0.168.192 -> 192.168.0.237

			assert(pNode != nullptr);

			// clear returning struct
			::std::memset(&::std::get<0>(aggregate), 0, sizeof(IPV4ADDR));		// ipaddr
			::std::get<1>(aggregate) = 0;											// prefix length
			::std::get<2>(aggregate).clear(); //INT = ::NM::ODB::INVALID_OBJECT_UID;						// interface uid

																				// check valid addr/prefix length
			PrefixType prefixType = GetPrefixType(ipv4, prefixLength);
			if (prefixType == PrefixType::Broadcast) return false;
			if (prefixType == PrefixType::Host) prefixLengthDepth = prefixLength; //32;


			// search down the tree to a max depth of prefixLength
			for (long i = 32 - 1; i >= (32 - prefixLengthDepth); i--)
			{
				++networkPrefixLengthCount;
				// get decimal value for this bit
				ipv4bitsb = 1L << i;

				// if this bit was set, set temp pointer to left or right child of the current node 
				// bit=1=right side, left(0) / right(1) bit testing
				if (hladdr & ipv4bitsb)
				{
					*pByte += 1 << (i % 8);
					pChildNode = pNode->_right;
				}
				else
				{
					pChildNode = pNode->_left;
				}

				// check we have valid child node on this side of the current node
				// and if not, terminate the search, given prefix not found, return failed
				if (!pChildNode)
				{
					// if user has submitted a prefix that might not be in the tree, they
					// maybe just checking for what would be the aggregate match for the given prefix
					// so, leave bFound status as is, else we flag not found - exit either way as we cant continue
					bAggregateFound = (ignoreMissingPrefix ? bAggregateFound : false);
					break;
				}

				// if we passed one byte (8bits) move pointer to next octet to sum octet value
				if (i % 8 == 0)
					pByte++;

				// save temp reference to this node if its a LPM, this is the last aggregate we saw for the given prefix.
				//if ((pChildNode->_interfaceUID != ::NM::ODB::INVALID_OBJECT_UID) && (i != 32 - prefixLengthDepth))
				if ((pChildNode->_prefixLength > 0) && (i != 32 - prefixLengthDepth))
				{
					pLastLPMNode = pChildNode;	// reference to current node
					// save copy of the current aggregate address, prefix and related interface uid
					::std::memcpy(&ipLastLPMbytes, &ipbytes, sizeof(unsigned __int8) * 4);
					lastLPMPrefixLength = networkPrefixLengthCount;
					GetInterfaces(pLastLPMNode, lastLPMInterfaceUID);
					//INT lastLPMInterfaceUID = pLastLPMNode->_interfaceUID;
					// set flag to found an aggregate
					bAggregateFound = true;
				}

				// set next node to process
				pNode = pChildNode;
			}

			// if the last node which was the last bit in the prefix; has an invalid objectuid, then 
			// the prefix was not found, the last bit is just part of a LPM chain. 
			// if the use has not set ignoreMissingPrefix=true, then we fail this search
			//INT if (pChildNode && (pChildNode->_interfaceUID == ::NM::ODB::INVALID_OBJECT_UID) && !ignoreMissingPrefix)
			if (pChildNode && (GetInterfaceCount(pChildNode) == 0) && !ignoreMissingPrefix)
				return nullptr;

			// if we found an aggregate
			if (bAggregateFound)
			{
				// copy LPM to returning function.
				::std::memcpy(&::std::get<0>(aggregate), &ipLastLPMbytes, sizeof(unsigned __int8) * 4);	// ipaddr
				::std::get<1>(aggregate) = lastLPMPrefixLength;											// prefix length
				::std::get<2>(aggregate) = lastLPMInterfaceUID;											// interface uid
			}
			return pLastLPMNode;
		}
		/**
		* InsertInterface
		*
		* Creates a new list if one does not exist. Adds the given objectuid to the list(set)
		*/
		void PTree::InsertInterface(PTNode* pNode, ::NM::ODB::OBJECTUID objectUID)
		{
			assert(objectUID != ::NM::ODB::INVALID_OBJECT_UID);
			if (!pNode->_pInterfaceList)
				pNode->_pInterfaceList = new INTFLIST;

			pNode->_pInterfaceList->insert(objectUID);
			return;
		}
		/**
		* DeleteInterface
		*
		* Deletes given objectuid if exists, it the new size of the interface list is 0, deletes the interface list
		* Where objectUID = ::NM::ODB::INVALID_OBJECT_UID, all current interfaces will be removed
		* returns remaining interface count 
		*/
		size_t PTree::DeleteInterface(RD rd, IPV4ADDR& ipv4Address, IPV4PREFIXLENGTH prefixLength, ::NM::ODB::OBJECTUID objectUID = ::NM::ODB::INVALID_OBJECT_UID)
		{
			size_t intfSize = 0;
			PTNode* rdNode = FindRouteDistinguisher(rd);
			if (!rdNode) return 0;

			::NM::IPRTREE::IPV4ADDR ipv4Network;
			ipv4Network.S_un.S_addr = 0;
			::NM::IPRTREE::IPV4PREFIXLENGTH ipv4PrefixLength = 0;
			PTNode* pNode = FindIPv4Address(rdNode, ipv4Address, prefixLength, ::NM::IPRTREE::MatchType::Exact, ipv4Network, ipv4PrefixLength);
			assert(pNode);
			if (!pNode) return 0;

			// remove existing interface entry from tree and cleanup if required to ensure structure integirty
			INTFLIST::iterator it = pNode->_pInterfaceList->find(objectUID);
			if (it != pNode->_pInterfaceList->end())
			{
				pNode->_pInterfaceList->erase(it);
				intfSize = pNode->_pInterfaceList->size();
			}

			if ((pNode->_pInterfaceList->size() == 0) || (objectUID == ::NM::ODB::INVALID_OBJECT_UID))
			{
				delete pNode->_pInterfaceList;
				pNode->_pInterfaceList = nullptr;
				intfSize = 0;
			}

			if (intfSize == 0)
			{

				::NM::IPRTREE::PrefixType prefixType = GetPrefixType(ipv4Address, prefixLength);
				assert(prefixType != PrefixType::Broadcast);

				if (prefixType == PrefixType::Network)
				{
					// if node has children
					if (pNode->_left || pNode->_right)
					{
						//	dont delete, nothing to change, leave prefixlength as its a network marker
					}
					// has no children
					else
					{
						// *** then delete node and prune back to first branch
						// get the last branch before the end of the prefixlength
						PTNode* pParentNode = nullptr;
						PTNode* pBranchChild = nullptr;
						FindLastBranch(rdNode, ipv4Address, prefixLength, &pParentNode, &pBranchChild);
						assert(pParentNode && pBranchChild);
						// thought, poss no pBranchChild !
						if ((!pParentNode) || (!pBranchChild))
							return 0;

						// delete branch starting from pBranchChild children
						DeleteRecursive(pBranchChild);

						// set the child pointer of the parent to null, will be one or the other
						if (pParentNode->_left == pBranchChild)
							pParentNode->_left = nullptr;
						else if (pParentNode->_right == pBranchChild)
							pParentNode->_right = nullptr;
						else
							throw ::std::runtime_error("PTree::Delete\tsomething gone wrong...");

						// delete pBranchChild
						DeleteNode(pBranchChild);

					}
				}
				// else its a host or hostroute
				else
				{
					// if has children 
					if (pNode->_left || pNode->_right)
					{
						// has children, so set prefixlength=0 and dont delete/prune back
						pNode->_prefixLength = 0;
					}
					// has no children
					else
					{
						// *** then delete node and prune back to first branch
						// get the last branch before the end of the prefixlength
						PTNode* pParentNode = nullptr;
						PTNode* pBranchChild = nullptr;
						FindLastBranch(rdNode, ipv4Address, prefixLength, &pParentNode, &pBranchChild);
						assert(pParentNode && pBranchChild);
						// thought, poss no pBranchChild !
						if ((!pParentNode) || (!pBranchChild))
							return 0;

						// delete branch starting from pBranchChild children
						DeleteRecursive(pBranchChild);

						// set the child pointer of the parent to null, will be one or the other
						if (pParentNode->_left == pBranchChild)
							pParentNode->_left = nullptr;
						else if (pParentNode->_right == pBranchChild)
							pParentNode->_right = nullptr;
						else
							throw ::std::runtime_error("PTree::Delete\tsomething gone wrong...");

						// delete pBranchChild
						DeleteNode(pBranchChild);
					}
				}

			}
			return intfSize;
		}
		/**
		* HasInterface
		*
		* Returns true if the object uid exists for this node/prefix
		*/
		bool PTree::HasInterface(PTNode* pNode, ::NM::ODB::OBJECTUID objectUID)
		{
			if (!pNode->_pInterfaceList)
				return false;

			INTFLIST::iterator it = pNode->_pInterfaceList->find(objectUID);
			return (it != pNode->_pInterfaceList->end());
		}
		/**
		* GetInterfaces
		*
		* Copies interface list of this node/prefix to the given interfacelist, clears given list first
		*/
		void PTree::GetInterfaces(PTNode* pNode, INTFLIST& interfaceList)
		{
			if (!pNode->_pInterfaceList)
			{
				interfaceList.clear();
				return;
			}
			interfaceList = *(pNode->_pInterfaceList);
			return;
		}
		/**
		* DeleteNode
		*
		*
		* Safely deletes a tree node.
		*/
		void PTree::DeleteNode(PTNode* pNode)
		{
			if (pNode->_pInterfaceList)
			{
				delete pNode->_pInterfaceList;
				pNode->_pInterfaceList = nullptr;
			}

			delete pNode;
			pNode = nullptr;

			return;
		}
		/**
		*
		*
		*
		*/
		size_t PTree::GetInterfaceCount(PTNode* pNode)
		{
			return (!pNode->_pInterfaceList ? 0 : pNode->_pInterfaceList->size());
		}
		/**
		*
		*
		*
		*/
		void PTree::DatabaseUpdate()
		{
			while (!_pUpdateQueue->Empty())
			{
				::std::shared_ptr<::NM::ODB::DatabaseUpdateRecord> record = _pUpdateQueue->GetNextRecord();
				::NM::ODB::DatabaseUpdateType updateType = record->GetUpdateType();
				::NM::ODB::OBJECTUID objectUID = record->GetObjectUID();
				switch (updateType)
				{

				case ::NM::ODB::DatabaseUpdateType::Create:
				{					
					switch (record->GetObjectType())
					{
					case ::NM::ODB::ObjectType::Interface:
					{
						InsertPrefix(record);
					}
					break;

					default:
						break;
					}
				}
				break;  // case ::NM::ODB::DatabaseUpdateType::Create:



				case ::NM::ODB::DatabaseUpdateType::Delete:
				{
					switch (record->GetObjectType())
					{
					case ::NM::ODB::ObjectType::Interface:
					{
						DeletePrefix(record);
					}
					break;

					default:
						break;
					}
				}
				break;  // case ::NM::ODB::DatabaseUpdateType::Delete:



				case ::NM::ODB::DatabaseUpdateType::Update:
				{
					switch (record->GetObjectType())
					{

					case ::NM::ODB::ObjectType::Interface:
					{
						UpdatePrefix(record);
					}
					break;

					default:
						break;
					}
				}
				break;  // case ::NM::ODB::DatabaseUpdateType::Update:



				case ::NM::ODB::DatabaseUpdateType::Refresh:
				{
					RefreshRadixTreeData();					
				}
				break;	// case ::NM::ODB::DatabaseUpdateType::Refresh:
				

				default:
					break;
				}

			}
		}
		/**
		*
		*
		*
		*/
		void PTree::RefreshRadixTreeData()
		{
			ClearRadixTreeData();
			// loop through all interfaces
			::std::wstring table = L"interfacetable";
			::NM::ODB::OBJECTUID objectUID = _odb->GetFirstObject(table);
			while (objectUID != ::NM::ODB::INVALID_OBJECT_UID)
			{
				// grab the ip address
				::NM::ODB::ODBLongLong rd = _odb->GetValue(objectUID, L"rd")->Get<::NM::ODB::ODBLongLong>();
				::NM::ODB::ODBINADDR intfIPv4Address = _odb->GetValue(objectUID, L"ipaddress")->Get<::NM::ODB::ODBINADDR>();
				::NM::ODB::ODBByte intfIPv4PrefixLength = _odb->GetValue(objectUID, L"ipprefixlength")->Get<::NM::ODB::ODBByte>();

				// insert into radix tree
				Insert(RDLONGTOASNT0(rd), RDLONGTOVALUET0(rd), intfIPv4Address, intfIPv4PrefixLength, objectUID);

				objectUID = _odb->GetNextObject(objectUID);
			}
			return;
		}
		/**
		*
		*
		*
		*/
		void PTree::InsertPrefix(::std::shared_ptr<::NM::ODB::DatabaseUpdateRecord> record)
		{
			::NM::ODB::OBJECTUID objectUID = record->GetObjectUID();
			::NM::IPRTREE::RD rd = _odb->GetValue(objectUID, L"rd")->Get<::NM::ODB::ODBLongLong>();
			PTNode* rdNode = FindRouteDistinguisher(rd);
			if (!rdNode) return;
			::NM::ODB::ODBINADDR ipAddr = _odb->GetValue(objectUID, L"ipaddress")->Get<::NM::ODB::ODBINADDR>();
			::NM::IPRTREE::IPV4PREFIXLENGTH prefixLength = _odb->GetValue(objectUID, L"ipprefixlength")->Get<::NM::ODB::ODBByte>();
			Insert(RDLONGTOASNT0(rd), RDLONGTOVALUET0(rd), ipAddr, prefixLength, objectUID);			
			return;
		}
		/**
		*
		*
		*
		*/
		void PTree::UpdatePrefix(::std::shared_ptr<::NM::ODB::DatabaseUpdateRecord> record)
		{
			::NM::ODB::OBJECTUID objectUID = record->GetObjectUID();

			// rd changed
			if (record->GetObjectAttributeName() == L"rd")
			{
				// get existing details
				::NM::IPRTREE::RD previousRD = record->GetPreviousValue().Get<::NM::ODB::ODBLongLong>();
				::NM::ODB::ODBINADDR ipAddr = _odb->GetValue(objectUID, L"ipaddress")->Get<::NM::ODB::ODBINADDR>();
				::NM::IPRTREE::IPV4PREFIXLENGTH prefixLength = _odb->GetValue(objectUID, L"ipprefixlength")->Get<::NM::ODB::ODBByte>();
				// remove interface from old ip address to be deleted
				DeleteInterface(previousRD, ipAddr, prefixLength, objectUID);
				// add new
				::NM::IPRTREE::RD newrd = record->GetNewValue().Get<::NM::ODB::ODBLongLong>();
				Insert(RDLONGTOASNT0(newrd), RDLONGTOVALUET0(newrd), ipAddr, prefixLength, objectUID);
			}
			// ip address changed
			else if (record->GetObjectAttributeName() == L"ipaddress")
			{
				::NM::IPRTREE::RD rd = _odb->GetValue(objectUID, L"rd")->Get<::NM::ODB::ODBLongLong>();
				::NM::IPRTREE::IPV4ADDR ipv4NetworkPrevious = record->GetPreviousValue().Get<::NM::ODB::ODBINADDR>();
				::NM::IPRTREE::IPV4PREFIXLENGTH prefixLength = _odb->GetValue(objectUID, L"ipprefixlength")->Get<::NM::ODB::ODBByte>();
				// remove interface from old ip address to be deleted
				DeleteInterface(rd, ipv4NetworkPrevious, prefixLength, objectUID);
				// insert updated prefix
				Insert(RDLONGTOASNT0(rd), RDLONGTOVALUET0(rd), record->GetNewValue().Get<::NM::ODB::ODBINADDR>(), prefixLength, objectUID);

			}
			// ip prefix length changed
			else if (record->GetObjectAttributeName() == L"ipprefixlength")
			{
				// get current RD/IPADDR/PREFIX before change
				::NM::IPRTREE::RD rd = _odb->GetValue(objectUID, L"rd")->Get<::NM::ODB::ODBLongLong>();
				::NM::ODB::ODBINADDR ipAddr = _odb->GetValue(objectUID, L"ipaddress")->Get<::NM::ODB::ODBINADDR>();
				::NM::IPRTREE::IPV4PREFIXLENGTH previousPrefixLength = record->GetPreviousValue().Get<::NM::ODB::ODBByte>();
				// remove interface from old ip prefix to be changed
				DeleteInterface(rd, ipAddr, previousPrefixLength, objectUID);
				// insert updated prefix
				Insert(RDLONGTOASNT0(rd), RDLONGTOVALUET0(rd), ipAddr, record->GetNewValue().Get<::NM::ODB::ODBByte>(), objectUID);
			}			
			return;
		}
		/**
		*
		*
		*
		*/
		void PTree::DeletePrefix(::std::shared_ptr<::NM::ODB::DatabaseUpdateRecord> record)
		{
			::NM::ODB::OBJECTUID objectUID = record->GetObjectUID();
			::NM::IPRTREE::RD rd = _odb->GetValue(objectUID, L"rd")->Get<::NM::ODB::ODBLongLong>();
			PTNode* rdNode = FindRouteDistinguisher(rd);
			if (!rdNode) return;
			::NM::ODB::ODBINADDR ipAddr = _odb->GetValue(objectUID, L"ipaddress")->Get<::NM::ODB::ODBINADDR>();
			::NM::IPRTREE::IPV4PREFIXLENGTH prefixLength = _odb->GetValue(objectUID, L"ipprefixlength")->Get<::NM::ODB::ODBByte>();
			DeleteInterface(rd, ipAddr, prefixLength, objectUID);
			return;
		}
		/**
		* do we need this
		*
		*
		*/
		bool PTree::InterfaceDeleteTrigger(::NM::ODB::IObjectDatabase&, ::NM::ODB::OBJECTUID, const ::NM::ODB::Value& pValue, const ::NM::ODB::Value& nValue)
		{
			return true;
		}
		/**
		* do we need this 
		*
		*
		*/
		bool PTree::InterfaceInsertTrigger(::NM::ODB::IObjectDatabase&, ::NM::ODB::OBJECTUID, const ::NM::ODB::Value& pValue, const ::NM::ODB::Value& nValue)
		{
			return true;
		}
		/**
		* IPv4AddressUpdateTrigger
		*
		* Checks to ensure the new ip address is valid host, hostroute or network. i.e. no broadcast address allowed to be assigned.
		*/
		bool PTree::IPv4AddressUpdateTrigger(::NM::ODB::IObjectDatabase&, ::NM::ODB::OBJECTUID objectUID, const ::NM::ODB::Value& pValue, const ::NM::ODB::Value& nValue)
		{
			// check to see if the new address is consistent with a host, hostroute or network, no other type allowed - i.e. broadcast

			// get prefixlength
			IPV4PREFIXLENGTH prefixLength = _odb->GetValue(objectUID, L"ipprefixlength")->Get<::NM::ODB::ODBByte>();

			PrefixType prefixType = GetPrefixType(nValue.Get<::NM::ODB::ODBINADDR>(), prefixLength);

			switch (prefixType)
			{
			case PrefixType::Host:
			case PrefixType::HostRoute:
			case PrefixType::Network:
				return true;
				break;

			default:
				return false;
			}

			return false;
		}
		/**
		* IPv4PrefixLengthUpdateTrigger
		*
		* Checks to ensure the new prefix length leaves a valid host, hostroute or network. i.e. no broadcast address allowed to be assigned.
		*/
		bool PTree::IPv4PrefixLengthUpdateTrigger(::NM::ODB::IObjectDatabase&, ::NM::ODB::OBJECTUID objectUID, const ::NM::ODB::Value& pValue, const ::NM::ODB::Value& nValue)
		{

			IPV4PREFIXLENGTH newPrefixLength = nValue.Get<::NM::ODB::ODBByte>();

			if ((newPrefixLength < 0) || (newPrefixLength > 32)) return false;

			// get ipaddress 
			IPV4ADDR ipaddr = _odb->GetValue(objectUID, L"ipaddress")->Get<::NM::ODB::ODBINADDR>();

			PrefixType prefixType = GetPrefixType(ipaddr, newPrefixLength);

			switch (prefixType)
			{
			case PrefixType::Host:
			case PrefixType::HostRoute:
			case PrefixType::Network:
				return true;
				break;

			default:
				return false;
			}

			return false;
		}


		// ns
	}
}