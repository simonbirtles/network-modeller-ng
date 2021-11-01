#pragma once
#include "..\..\ServiceRegistry\ServiceBase.h"

#include <memory>

namespace NM
{
	namespace DataServices
	{
		class CAdjacencyMatrix;

		class AdjacencyMatrixService : public ::NM::Registry::ServiceBase
		{
		public:
			AdjacencyMatrixService();
			~AdjacencyMatrixService();

			// Registry virtuals
			bool StartRegService();
			bool StopRegService();
			void* GetClientInterface();

		private:
			::std::shared_ptr<CAdjacencyMatrix> _pMatrix;
		};


	}
}
