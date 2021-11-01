#include "stdafx.h"
#include "ITrafficGraph.h"

namespace NM
{
	namespace ODB
	{
		ITrafficGraph::ITrafficGraph(TrafficGraph** ppTrafficGraph):
			_trafficGraph(ppTrafficGraph)
		{
		}


		ITrafficGraph::~ITrafficGraph(void)
		{
		}


	}
}