#pragma once

#include <Engine.Core/Module/SubModule.hpp>

namespace hg::engine::reflow::theming {
	class Theming;
}

namespace hg::engine {
	class Reflow :
		public core::SubModule {
	public:
		using this_type = Reflow;

	public:
		Reflow(const non_owning_rptr<Engine> engine_);

		~Reflow() override;

	private:
		CompactSet<core::SubModuleDependency> _dependencies;

	public:
		[[nodiscard]] core::DependencyKey moduleKey() const noexcept override;

		[[nodiscard]] cref<CompactSet<core::SubModuleDependency>> dependencies() const noexcept override;

	public:
		void setup() override;

		void start() override;

		void stop() override;

		void destroy() override;

	private:
		UniquePtr<reflow::theming::Theming> _theming;

	public:
		[[nodiscard]] ref<const reflow::theming::Theming> getTheming() const noexcept;

		[[nodiscard]] ref<reflow::theming::Theming> getTheming() noexcept;
	};
}
