#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Core;
#endif

namespace hg::engine::acs {
	class Registry;
}

namespace hg::engine {
	class ActorModule final :
		public core::CoreModule {
	private:
		friend class ::hg::engine::core::ModuleSlotAccess;

	public:
		using this_type = ActorModule;

		using ActorRegistry = ::hg::engine::acs::Registry;

	public:
		explicit ActorModule(ref<Engine> engine_);

		ActorModule(cref<this_type>) = delete;

		ActorModule(mref<this_type>) = delete;

		~ActorModule() override;

	private:
		static constinit std::atomic<ptr<ActorModule>> _moduleSlot;

	public:
		[[nodiscard]] static Opt<ref<ActorModule>> get() noexcept {
			return engine::module<ActorModule>();
		}

	public:
		void setup() override;

		void start() override;

		void stop() override;

		void destroy() override;

	private:
		uptr<ActorRegistry> _registry;

	public:
		[[nodiscard]] nmpt<ActorRegistry> getRegistry() const noexcept;
	};
}
