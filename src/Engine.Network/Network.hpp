#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Core;
#endif

namespace hg::engine {
	class Network :
		public core::CoreModule {
	private:
		friend class ::hg::engine::core::ModuleSlotAccess;

	public:
		Network(ref<Engine> engine_) noexcept;

		~Network() override;

	private:
		static constinit std::atomic<ptr<Network>> _moduleSlot;

	public:
		[[nodiscard]] static Opt<ref<Network>> get() noexcept {
			return engine::module<Network>();
		}

	public:
		void setup() override;

		void start() override;

		void stop() override;

		void destroy() override;
	};
}
