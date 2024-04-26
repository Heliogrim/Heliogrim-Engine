#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

namespace hg::engine {
	class Engine;
}

namespace hg::engine::core {
	class __declspec(novtable) CoreModule {
	public:
		CoreModule(nmpt<Engine> engine_) :
			_engine(engine_) {}

		virtual ~CoreModule() = default;

	protected:
		nmpt<Engine> _engine;

	public:
		virtual void setup() = 0;

		virtual void start() = 0;

		virtual void stop() = 0;

		virtual void destroy() = 0;
	};
}
