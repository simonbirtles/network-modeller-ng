#pragma once

namespace NM
{
	namespace Registry
	{

		typedef size_t GRAPHID;			

		enum class ServiceType {Real, Phantom, Invalid};
		enum class ServiceStatus { Running, Stopped, Unknown };

		#define __STR2__(x) #x
		#define __STR1__(x) __STR2__(x)
		#define __LOC__ __FILE__ "("__STR1__(__LINE__)") : Warning Msg: "

	}
}
