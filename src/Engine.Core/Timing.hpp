 #pragma once

#include <chrono>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

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
	};
}
