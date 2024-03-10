#pragma once

#include <Engine.Common/Sal.hpp>

#include "Fiber.hpp"

namespace hg::engine::scheduler::fiber {
	struct FiberLaunchPad {
	public:
		ptr<Fiber> self;

	public:
		void operator()();

		static void __stdcall launch(_In_ ptr<void> self_);
	};
}
