#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Set.hpp>

#include "DependencyKey.hpp"
#include "SubModuleDependency.hpp"

namespace hg::engine {
	class Engine;
}

namespace hg::engine::core {
	class __declspec(novtable) SubModule {
	public:
		explicit constexpr SubModule(const non_owning_rptr<Engine> engine_) :
			_engine(engine_) {}

		constexpr virtual ~SubModule() = default;

	protected:
		const non_owning_rptr<Engine> _engine;

	public:
		[[nodiscard]] virtual DependencyKey moduleKey() const noexcept = 0;

		[[nodiscard]] virtual cref<CompactSet<SubModuleDependency>> dependencies() const noexcept = 0;

	public:
		virtual void setup() = 0;

		virtual void start() = 0;

		virtual void stop() = 0;

		virtual void destroy() = 0;
	};
}
