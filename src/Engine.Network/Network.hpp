#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Core/Module/CoreModule.hpp>

namespace hg::engine {
	class Network :
		public core::CoreModule {
	public:
		Network(ref<Engine> engine_) noexcept;

		~Network() override;

	public:
		void setup() override;

		void start() override;

		void stop() override;

		void destroy() override;
	};
}
