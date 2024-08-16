#pragma once

#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Deque.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Reflect/Meta/TypeId.hpp>

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
		[[nodiscard]] Optional<ref<ModuleType_>> getRootModule() const noexcept;

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
		[[nodiscard]] Optional<ref<ModuleType_>> getCoreModule() const noexcept;

		template <IsCoreModule ModuleType_>
		[[nodiscard]] Optional<ref<ModuleType_>> getCoreModuleUnsafe(_In_ cref<DependencyKey> key_) const noexcept;

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
		addRootModule(reflect::typeId<ModuleType_>(), std::addressof(module_));
	}

	template <IsRootModule ModuleType_>
	Optional<ref<ModuleType_>> Modules::getRootModule() const noexcept {
		auto val = getRootModule(reflect::typeId<ModuleType_>());
		return (val != nullptr) ? Optional { static_cast<ref<ModuleType_>>(*val) } : Optional { tl::nullopt };
	}

	template <IsRootModule ModuleType_>
	void Modules::removeRootModule(ref<ModuleType_> module_) {
		removeRootModule(reflect::typeId<ModuleType_>(), std::addressof(module_));
	}

	/**/

	template <IsCoreModule ModuleType_>
	void Modules::addCoreModule(ref<ModuleType_> module_, cref<DependencyKey> key_) {
		addCoreModule(std::move(key_), reflect::typeId<ModuleType_>(), std::addressof(module_));
	}

	template <IsCoreModule ModuleType_>
	Optional<ref<ModuleType_>> Modules::getCoreModule() const noexcept {
		auto val = getCoreModule(reflect::typeId<ModuleType_>());
		return (val != nullptr) ? Optional { static_cast<ref<ModuleType_>>(*val) } : Optional { tl::nullopt };
	}

	template <IsCoreModule ModuleType_>
	Optional<ref<ModuleType_>> Modules::getCoreModuleUnsafe(cref<DependencyKey> key_) const noexcept {
		auto val = getCoreModule(key_);
		return (val != nullptr) ? Optional { static_cast<ref<ModuleType_>>(*val) } : Optional { tl::nullopt };
	}

	template <IsCoreModule ModuleType_>
	void Modules::removeCoreModule(ref<ModuleType_> module_) {
		removeCoreModule(reflect::typeId<ModuleType_>(), std::addressof(module_));
	}
}
