#pragma once

#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Deque.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Reflect/TypeId.hpp>

#include "CoreModule.hpp"
#include "DependencyKey.hpp"
#include "DependencyValidationResult.hpp"
#include "RootModule.hpp"
#include "SubModule.hpp"

namespace hg::engine::core {
	class Modules {
	public:
		using this_type = Modules;

	public:
		Modules() noexcept = default;

		Modules(cref<this_type>) = delete;

		Modules(mref<this_type>) noexcept = delete;

		~Modules();

	private:
		struct RootModuleEntry {
			type_id baseTypeId;
			nmpt<RootModule> module;
		};

		struct CoreModuleEntry {
			DependencyKey key;
			type_id baseTypeId;
			nmpt<CoreModule> module;
		};

	private:
		Vector<RootModuleEntry> _rootModules;
		Vector<CoreModuleEntry> _coreModules;
		Deque<uptr<SubModule>> _subModules;

	private:
		[[nodiscard]] bool validateDependency(cref<SubModuleDependency> dependency_) const noexcept;

	private:
		void addRootModule(_In_ mref<type_id> baseTypeId_, _In_ nmpt<RootModule> rootModule_);

		[[nodiscard]] nmpt<RootModule> getRootModule(mref<type_id> baseTypeId_) const noexcept;

		void removeRootModule(_In_ mref<type_id> baseTypeId_, _In_ nmpt<RootModule> rootModule_);

	public:
		[[nodiscard]] size_t getRootModuleCount() const noexcept;

		template <IsRootModule ModuleType_>
		void addRootModule(_In_ ref<ModuleType_> module_);

		template <IsRootModule ModuleType_>
		[[nodiscard]] Opt<ref<ModuleType_>> getRootModule() const noexcept;

		template <IsRootModule ModuleType_>
		void removeRootModule(_In_ ref<ModuleType_> module_);

	private:
		void addCoreModule(
			_In_ cref<DependencyKey> key_,
			_In_ mref<type_id> baseTypeId_,
			_In_ nmpt<CoreModule> coreModule_
		);

		[[nodiscard]] nmpt<CoreModule> getCoreModule(
			cref<decltype(std::declval<decltype(_coreModules)::value_type>().key)> key_
		) const noexcept;

		[[nodiscard]] nmpt<CoreModule> getCoreModule(
			mref<decltype(std::declval<decltype(_coreModules)::value_type>().baseTypeId)> baseTypeId_
		) const noexcept;

		void removeCoreModule(
			_In_ mref<type_id> baseTypeId_,
			_In_ nmpt<CoreModule> coreModule_
		);

	public:
		[[nodiscard]] size_t getCoreModuleCount() const noexcept;

		template <IsCoreModule ModuleType_>
		void addCoreModule(_In_ ref<ModuleType_> module_, _In_ cref<DependencyKey> key_);

		template <IsCoreModule ModuleType_>
		[[nodiscard]] Opt<ref<ModuleType_>> getCoreModule() const noexcept;

		template <IsCoreModule ModuleType_>
		[[nodiscard]] Opt<ref<ModuleType_>> getCoreModuleUnsafe(_In_ cref<DependencyKey> key_) const noexcept;

		void forEachCoreModule(auto&& fn_) const requires std::is_invocable_v<decltype(fn_), ref<CoreModule>> {
			for (const auto& coreModuleEntry : _coreModules) {
				fn_(*coreModuleEntry.module.get());
			}
		}

		template <IsCoreModule ModuleType_>
		void removeCoreModule(_In_ ref<ModuleType_> module_);

	public:
		[[nodiscard]] size_t getSubModuleCount() const noexcept;

		DependencyValidationResult addSubModule(mref<uptr<SubModule>> subModule_);

		[[nodiscard]] nmpt<SubModule> getSubModule(cref<DependencyKey> moduleKey_) const noexcept;

		[[nodiscard]] cref<Deque<uptr<SubModule>>> getSubModules() const noexcept;

		bool removeSubModule(nmpt<SubModule> subModule_);

	public:
		[[nodiscard]] DependencyValidationResult validate() const noexcept;
	};

	/**/

	template <IsRootModule ModuleType_>
	void Modules::addRootModule(ref<ModuleType_> module_) {
		addRootModule(::hg::refl::TypeId<ModuleType_>(), std::addressof(module_));
	}

	template <IsRootModule ModuleType_>
	Opt<ref<ModuleType_>> Modules::getRootModule() const noexcept {
		auto val = getRootModule(::hg::refl::TypeId<ModuleType_>());
		return (val != nullptr) ? Some(static_cast<ref<ModuleType_>>(*val)) : None;
	}

	template <IsRootModule ModuleType_>
	void Modules::removeRootModule(ref<ModuleType_> module_) {
		removeRootModule(::hg::refl::TypeId<ModuleType_>(), std::addressof(module_));
	}

	/**/

	template <IsCoreModule ModuleType_>
	void Modules::addCoreModule(ref<ModuleType_> module_, cref<DependencyKey> key_) {
		addCoreModule(std::move(key_), ::hg::refl::TypeId<ModuleType_>(), std::addressof(module_));
	}

	template <IsCoreModule ModuleType_>
	Opt<ref<ModuleType_>> Modules::getCoreModule() const noexcept {
		auto val = getCoreModule(::hg::refl::TypeId<ModuleType_>());
		return (val != nullptr) ? Some(static_cast<ref<ModuleType_>>(*val)) : None;
	}

	template <IsCoreModule ModuleType_>
	Opt<ref<ModuleType_>> Modules::getCoreModuleUnsafe(cref<DependencyKey> key_) const noexcept {
		auto val = getCoreModule(key_);
		return (val != nullptr) ? Some(static_cast<ref<ModuleType_>>(*val)) : None;
	}

	template <IsCoreModule ModuleType_>
	void Modules::removeCoreModule(ref<ModuleType_> module_) {
		removeCoreModule(::hg::refl::TypeId<ModuleType_>(), std::addressof(module_));
	}
}
