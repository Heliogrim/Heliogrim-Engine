#pragma once

#include <Engine.Common/__macro.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::core {
	class Timing final {
	public:
		Timing() noexcept;

		Timing(ref<Timing>) = delete;

		Timing(mref<Timing>) = delete;

		~Timing() noexcept;

	private:
		static Opt<ref<Timing>> _cached;

		std::chrono::high_resolution_clock::time_point _lastTimePoint;
		f32 _rawDeltaTime;

	public:
		void updateTimings();

	public:
		[[nodiscard]] static f32 getDeltaTime() noexcept;

		[[nodiscard]] static f32 getDeltaTime(std::chrono::seconds) noexcept;
	};
}
