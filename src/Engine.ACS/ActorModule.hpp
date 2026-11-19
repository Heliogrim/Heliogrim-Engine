#pragma once

#include <Engine.Common/__macro.hpp>
#include <Engine.Core/Module/CoreModule.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::acs {
	class Registry;
}

namespace hg::engine {
	class ActorModule final :
		public core::CoreModule {
	public:
		using this_type = ActorModule;

		using ActorRegistry = ::hg::engine::acs::Registry;

	public:
		explicit ActorModule(ref<Engine> engine_);

		ActorModule(cref<this_type>) = delete;

		ActorModule(mref<this_type>) = delete;

		~ActorModule() override;

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
