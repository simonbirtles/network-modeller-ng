#pragma once

namespace NM
{
	namespace ODB
	{
		class TrafficGraph;

		class ITrafficGraph
		{
		public:
			explicit ITrafficGraph(TrafficGraph** ppTrafficGraph);
			~ITrafficGraph(void);

		private:
			TrafficGraph**	_trafficGraph;

		};

	}
}

