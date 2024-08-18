#pragma once

#include <Engine.Common/Sal.hpp>

#include "Fiber.hpp"

namespace hg::engine::scheduler::fiber {
	struct FiberLaunchPad {
	public:
		ptr<Fiber> self;

	public:
		void operator()();

		#if defined(WIN32)
		static void launch(_In_ ptr<void> self_);
		#else
		static void launch(_In_ int ptrLow_, _In_ int ptrHigh_);
		#endif
	};
}
