#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

namespace hg::engine {
	class Engine;
}

namespace hg::engine::core {
	class CoreModule;

	template <class Type_>
	concept IsCoreModule = std::derived_from<Type_, CoreModule>;

	/**/

	class macro_novtable CoreModule {
	public:
		explicit constexpr CoreModule(ref<Engine> engine_) :
			_engine(std::addressof(engine_)) {}

		constexpr virtual ~CoreModule() = default;

	protected:
		nmpt<Engine> _engine;

	public:
		virtual void setup() = 0;

		virtual void start() = 0;

		virtual void stop() = 0;

		virtual void destroy() = 0;
	};
}
