#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Deque.hpp>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "DependencyKey.hpp"
#include "DependencyValidationResult.hpp"

namespace hg::engine::core {
	class SubModule;
	struct SubModuleDependency;
}

namespace hg::engine::core {
	class Modules {
	public:
		using this_type = Modules;

	public:
		Modules();

		Modules(cref<this_type>) = delete;

		Modules(mref<this_type>) noexcept = delete;

		~Modules();

	private:
		Deque<uptr<SubModule>> _subModules;
		CompactSet<DependencyKey> _coreModules;

	private:
		[[nodiscard]] bool validateDependency(cref<SubModuleDependency> dependency_) const noexcept;

	public:
		[[nodiscard]] size_t getCoreModuleCount() const noexcept;

		[[nodiscard]] size_t getSubModuleCount() const noexcept;

		[[nodiscard]] cref<Deque<uptr<SubModule>>> getSubModules() const noexcept;

	public:
		DependencyValidationResult addSubModule(mref<uptr<SubModule>> subModule_);

		[[nodiscard]] nmpt<SubModule> getSubModule(cref<DependencyKey> moduleKey_) const noexcept;

		bool removeSubModule(nmpt<SubModule> subModule_);

	public:
		[[nodiscard]] DependencyValidationResult validate() const noexcept;
	};
}
