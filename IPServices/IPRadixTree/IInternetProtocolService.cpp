/**
* IInternetProtocolService
*
* Service wrapper for internal registry to start, stop and get client interfaces on request.
*/
#include "stdafx.h"
#include "IInternetProtocolService.h"
#include "PTree.h"
#include "IIPService.h"

namespace NM
{
	namespace IPRTREE
	{
		/**
		*
		*
		*
		*/
		IInternetProtocolService::IInternetProtocolService()
		{
		}
		/**
		*
		*
		*
		*/
		IInternetProtocolService::~IInternetProtocolService()
		{
		}
		/**
		*
		*
		*
		*/
		bool IInternetProtocolService::StartRegService()
		{
			_pTreeSvc.reset(new PTree());
			//AddSampleIPTreeData();
			return true;
		}
		/**
		*
		*
		*
		*/
		bool IInternetProtocolService::StopRegService()
		{
			_pTreeSvc.reset();
			return true;
		}
		/**
		*
		*
		*
		*/
		void* IInternetProtocolService::GetClientInterface()
		{
			assert(_pTreeSvc);
			return (void*)new IIPService(_pTreeSvc);
		}

		/**
		* temp test data
		*
		//*
		//*/
		//void IInternetProtocolService::AddSampleIPTreeData()
		//{

		//	::NM::IPRTREE::IPV4PREFIXLENGTH prefixLength = 0;
		//	::NM::IPRTREE::IPV4ADDR ipa;
		//	::NM::IPRTREE::RDASN rdasn = 65010;
		//	::NM::IPRTREE::RDVALUE rdvalue = 10;


		//	// insert a few staic test cases

		//	// defualt route handling not impl. yet.
		//	/*ipa.S_un.S_un_b.s_b1 = 0;
		//	ipa.S_un.S_un_b.s_b2 = 0;
		//	ipa.S_un.S_un_b.s_b3 = 0;
		//	ipa.S_un.S_un_b.s_b4 = 0;
		//	prefixLength = 0;*/
		//	///tree.Insert(rdasn, rdvalue, ipa, prefixLength, 101010);



		//	/**
		//	* Insert standard networks
		//	*/
		//	ipa.S_un.S_un_b.s_b1 = 192;
		//	ipa.S_un.S_un_b.s_b2 = 0;
		//	ipa.S_un.S_un_b.s_b3 = 0;
		//	ipa.S_un.S_un_b.s_b4 = 0;
		//	prefixLength = 8;
		//	_pTreeSvc->Insert(rdasn, rdvalue, ipa, prefixLength, 28);

		//	ipa.S_un.S_un_b.s_b1 = 192;
		//	ipa.S_un.S_un_b.s_b2 = 168;
		//	ipa.S_un.S_un_b.s_b3 = 0;
		//	ipa.S_un.S_un_b.s_b4 = 0;
		//	prefixLength = 16;
		//	_pTreeSvc->Insert(rdasn, rdvalue, ipa, prefixLength, 4);

		//	ipa.S_un.S_un_b.s_b1 = 192;
		//	ipa.S_un.S_un_b.s_b2 = 168;
		//	ipa.S_un.S_un_b.s_b3 = 32;
		//	ipa.S_un.S_un_b.s_b4 = 0;
		//	prefixLength = 24;
		//	_pTreeSvc->Insert(rdasn, rdvalue, ipa, prefixLength, 4);

		//	/**
		//	* Insert host address in above networks
		//	*/


		//	ipa.S_un.S_un_b.s_b1 = 192;
		//	ipa.S_un.S_un_b.s_b2 = 168;
		//	ipa.S_un.S_un_b.s_b3 = 32;
		//	ipa.S_un.S_un_b.s_b4 = 5;
		//	prefixLength = 24;
		//	_pTreeSvc->Insert(rdasn, rdvalue, ipa, prefixLength, 99999);



		//	/**
		//	* Insert host addresses with no existing network entry
		//	*/
		//	ipa.S_un.S_un_b.s_b1 = 192;
		//	ipa.S_un.S_un_b.s_b2 = 168;
		//	ipa.S_un.S_un_b.s_b3 = 125;
		//	ipa.S_un.S_un_b.s_b4 = 254;
		//	prefixLength = 30;
		//	_pTreeSvc->Insert(rdasn, rdvalue, ipa, prefixLength, 2);

		//	// add the other host in this subnet
		//	ipa.S_un.S_un_b.s_b1 = 192;
		//	ipa.S_un.S_un_b.s_b2 = 168;
		//	ipa.S_un.S_un_b.s_b3 = 125;
		//	ipa.S_un.S_un_b.s_b4 = 253;
		//	prefixLength = 30;
		//	_pTreeSvc->Insert(rdasn, rdvalue, ipa, prefixLength, 5);


		//	// add a random host route
		//	ipa.S_un.S_un_b.s_b1 = 192;
		//	ipa.S_un.S_un_b.s_b2 = 168;
		//	ipa.S_un.S_un_b.s_b3 = 72;
		//	ipa.S_un.S_un_b.s_b4 = 99;
		//	prefixLength = 32;
		//	_pTreeSvc->Insert(rdasn, rdvalue, ipa, prefixLength, 898);



		//	//ipa.S_un.S_un_b.s_b1 = 192;
		//	//ipa.S_un.S_un_b.s_b2 = 0;
		//	//ipa.S_un.S_un_b.s_b3 = 0;
		//	//ipa.S_un.S_un_b.s_b4 = 0;
		//	//prefixLength = 8;
		//	//_pTreeSvc->Insert(rdasn, rdvalue, ipa, prefixLength, 28);


		//	//ipa.S_un.S_un_b.s_b1 = 192;
		//	//ipa.S_un.S_un_b.s_b2 = 168;
		//	//ipa.S_un.S_un_b.s_b3 = 125;
		//	//ipa.S_un.S_un_b.s_b4 = 254;
		//	//prefixLength = 32;
		//	//_pTreeSvc->Insert(rdasn, rdvalue, ipa, prefixLength, 99999);


		//	//ipa.S_un.S_un_b.s_b1 = 192;
		//	//ipa.S_un.S_un_b.s_b2 = 168;
		//	//ipa.S_un.S_un_b.s_b3 = 125;
		//	//ipa.S_un.S_un_b.s_b4 = 254;
		//	//prefixLength = 30;
		//	//_pTreeSvc->Insert(rdasn, rdvalue, ipa, prefixLength, 2);


		//	//ipa.S_un.S_un_b.s_b1 = 192;
		//	//ipa.S_un.S_un_b.s_b2 = 168;
		//	//ipa.S_un.S_un_b.s_b3 = 125;
		//	//ipa.S_un.S_un_b.s_b4 = 254;
		//	//prefixLength = 28;
		//	//_pTreeSvc->Insert(rdasn, rdvalue, ipa, prefixLength, 3);


		//	//ipa.S_un.S_un_b.s_b1 = 192;
		//	//ipa.S_un.S_un_b.s_b2 = 168;
		//	//ipa.S_un.S_un_b.s_b3 = 0;
		//	//ipa.S_un.S_un_b.s_b4 = 0;
		//	//prefixLength = 16;
		//	//_pTreeSvc->Insert(rdasn, rdvalue, ipa, prefixLength, 4);

		//	//ipa.S_un.S_un_b.s_b1 = 192;
		//	//ipa.S_un.S_un_b.s_b2 = 23;
		//	//ipa.S_un.S_un_b.s_b3 = 82;
		//	//ipa.S_un.S_un_b.s_b4 = 0;
		//	//prefixLength = 24;
		//	//_pTreeSvc->Insert(rdasn, rdvalue, ipa, prefixLength, 10);

		//	//ipa.S_un.S_un_b.s_b1 = 192;
		//	//ipa.S_un.S_un_b.s_b2 = 128;
		//	//ipa.S_un.S_un_b.s_b3 = 56;
		//	//ipa.S_un.S_un_b.s_b4 = 0;
		//	//prefixLength = 22;
		//	//_pTreeSvc->Insert(rdasn, rdvalue, ipa, prefixLength, 11);

		//	///*
		//	//* Host Address Test
		//	//*/
		//	//ipa.S_un.S_un_b.s_b1 = 192;
		//	//ipa.S_un.S_un_b.s_b2 = 168;
		//	//ipa.S_un.S_un_b.s_b3 = 125;
		//	//ipa.S_un.S_un_b.s_b4 = 241;
		//	//prefixLength = 28;
		//	//_pTreeSvc->Insert(rdasn, rdvalue, ipa, prefixLength, 11);


		//	//ipa.S_un.S_un_b.s_b1 = 192;
		//	//ipa.S_un.S_un_b.s_b2 = 168;
		//	//ipa.S_un.S_un_b.s_b3 = 125;
		//	//ipa.S_un.S_un_b.s_b4 = 241;
		//	//prefixLength = 30;
		//	//_pTreeSvc->Insert(rdasn, rdvalue, ipa, prefixLength, 11);


		//	//ipa.S_un.S_un_b.s_b1 = 192;
		//	//ipa.S_un.S_un_b.s_b2 = 168;
		//	//ipa.S_un.S_un_b.s_b3 = 125;
		//	//ipa.S_un.S_un_b.s_b4 = 242;
		//	//prefixLength = 31;
		//	//_pTreeSvc->Insert(rdasn, rdvalue, ipa, prefixLength, 11);


		//	//ipa.S_un.S_un_b.s_b1 = 192;
		//	//ipa.S_un.S_un_b.s_b2 = 168;
		//	//ipa.S_un.S_un_b.s_b3 = 125;
		//	//ipa.S_un.S_un_b.s_b4 = 243;
		//	//prefixLength = 32;
		//	//_pTreeSvc->Insert(rdasn, rdvalue, ipa, prefixLength, 11);





		//	//ipa.S_un.S_un_b.s_b1 = 172;
		//	//ipa.S_un.S_un_b.s_b2 = 19;
		//	//ipa.S_un.S_un_b.s_b3 = 123;
		//	//ipa.S_un.S_un_b.s_b4 = 0;
		//	//prefixLength = 21;
		//	//_pTreeSvc->Insert(rdasn, rdvalue, ipa, prefixLength, 10);


		//	//ipa.S_un.S_un_b.s_b1 = 10;
		//	//ipa.S_un.S_un_b.s_b2 = 23;
		//	//ipa.S_un.S_un_b.s_b3 = 46;
		//	//ipa.S_un.S_un_b.s_b4 = 0;
		//	//prefixLength = 24;
		//	//_pTreeSvc->Insert(rdasn, rdvalue, ipa, prefixLength, 10);






		//	////ipa.S_un.S_un_b.s_b1 = 1;
		//	////ipa.S_un.S_un_b.s_b2 = 0;
		//	////ipa.S_un.S_un_b.s_b3 = 0;
		//	////ipa.S_un.S_un_b.s_b4 = 0;
		//	////prefixLength = 0;
		//	////_pTreeSvc->Insert(rdasn, rdvalue, ipa, prefixLength, 10);


		//	/////
		//	//ipa.S_un.S_un_b.s_b1 = 172;
		//	//ipa.S_un.S_un_b.s_b2 = 16;
		//	//ipa.S_un.S_un_b.s_b3 = 31;
		//	//ipa.S_un.S_un_b.s_b4 = 12;
		//	//prefixLength = 24;
		//	//_pTreeSvc->Insert(0, 0, ipa, prefixLength, 5);


		//	// insert some random test prefixes
		//	for (unsigned long i = 0; i < 1000; ++i)
		//	{
		//		ipa.S_un.S_un_b.s_b1 = (rand() % 3) + 10;					// 10. -> 12.
		//		ipa.S_un.S_un_b.s_b2 = rand() % 150;						// x.0- > x.150
		//		ipa.S_un.S_un_b.s_b3 = rand() % 255;						// x.y.0 -> x.y.255
		//		ipa.S_un.S_un_b.s_b4 = rand() % 255;						// x.y.z.0 -> x.y.z.255
		//		prefixLength = min(32, (rand() % 24) + 8);
		//		rdasn = 65000;												// one ASN (provider)
		//		rdvalue = (rand() % 100);// +200;							// 100 VPNs/Customers
		//								 /*OutputDebugString(::std::wstring(
		//								 L"\nInsert RD:\t" +
		//								 ::std::to_wstring(rdasn) +
		//								 L":" +
		//								 ::std::to_wstring(rdvalue) +
		//								 L":" +
		//								 ::std::to_wstring(ipa.S_un.S_un_b.s_b1) + L"." +
		//								 ::std::to_wstring(ipa.S_un.S_un_b.s_b2) + L"." +
		//								 ::std::to_wstring(ipa.S_un.S_un_b.s_b3) + L"." +
		//								 ::std::to_wstring(ipa.S_un.S_un_b.s_b4)
		//								 ).c_str());*/

		//								 // every now and again make sure we get a network in there
		//		if (rand() % 3 == 0)
		//		{
		//			_pTreeSvc->GetNetworkAddress(ipa, prefixLength, ipa);
		//		}

		//		_pTreeSvc->Insert(rdasn, rdvalue, ipa, prefixLength, i + 200);
		//	}

		//	return;

		//}

	}
}
