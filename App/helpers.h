#pragma once

namespace NM {

	namespace HELPERS {

		struct compareGuid
		{
			bool operator()(const ::NM::ODB::ODBUID& guid1, const::NM::ODB::ODBUID& guid2) const
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
				for (int i = 0; i < 8; i++) {
					if (guid1.Data4[i] != guid2.Data4[i]) {
						return guid1.Data4[i] < guid2.Data4[i];
					}
				}
				return false;
			}
		};
	}
}
// TODO: duplicated a lot - move to a common include for all.
