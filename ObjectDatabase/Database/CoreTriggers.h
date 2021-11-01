#pragma once
#include <memory>

namespace  NM
{
	namespace ODB
	{

		class TriggerManager;

		class CoreTriggers
		{
		public:
			static void CreateCoreTriggers(::std::shared_ptr<TriggerManager> triggerManager);

		private:
			CoreTriggers();
			~CoreTriggers();

			static void CreateCoreTrigger_f1(::std::shared_ptr<TriggerManager> triggerManager);
			static void CreateCoreTrigger_f2(::std::shared_ptr<TriggerManager> triggerManager);
			static void CreateCoreTrigger_f3(::std::shared_ptr<TriggerManager> triggerManager);
			static void CreateCoreTrigger_f4(::std::shared_ptr<TriggerManager> triggerManager);
		};


	}
}