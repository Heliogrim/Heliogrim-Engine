#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Scheduler;
#endif

namespace hg::engine {
	class TestCompScheduler final :
		public CompScheduler {
	public:
		TestCompScheduler() :
			CompScheduler() {}

		~TestCompScheduler() override = default;

	public:
		[[nodiscard]] ptr<scheduler::Schedule> testGetSchedule() const noexcept {
			return _schedule.get();
		}
	};
}
