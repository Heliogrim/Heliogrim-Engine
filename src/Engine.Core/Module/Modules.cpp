#include "Modules.hpp"

#include <algorithm>
#include <Engine.Asserts/Asserts.hpp>

#include "CoreDependencies.hpp"
#include "SubModule.hpp"
#include "SubModuleDependency.hpp"

using namespace hg::engine::core;
using namespace hg;

Modules::~Modules() = default;

bool Modules::validateDependency(cref<SubModuleDependency> dependency_) const noexcept {

	if (not std::ranges::contains(_coreModules, dependency_.key, [](const auto& entry_) { return entry_.key; })) {
		return true;
	}

	return dependency_.order != SubModuleOrder::ePredecessor;
}

size_t Modules::getRootModuleCount() const noexcept {
	return _rootModules.size();
}

void Modules::addRootModule(mref<type_id> baseTypeId_, nmpt<RootModule> rootModule_) {
	::hg::assertrt(
		std::ranges::find_if(
			_rootModules,
			[baseTypeId_](const auto& entry_) {
				return entry_.baseTypeId == baseTypeId_;
			}
		) == std::ranges::end(_rootModules)
	);
	_rootModules.emplace_back(std::move(baseTypeId_), std::move(rootModule_));
}

nmpt<RootModule> Modules::getRootModule(mref<type_id> baseTypeId_) const noexcept {
	const auto it = std::ranges::find_if(
		_rootModules,
		[baseTypeId_](const auto& entry_) {
			return entry_.baseTypeId == baseTypeId_;
		}
	);
	return (it != std::ranges::end(_rootModules)) ? it->module : nullptr;
}

void Modules::removeRootModule(mref<type_id> baseTypeId_, nmpt<RootModule> rootModule_) {

	// TODO: Validate internal dependency state, instead of force requiring non-coremodule/non-submodule state
	::hg::assertrt(_coreModules.empty());
	::hg::assertrt(_subModules.empty());

	const auto it = std::ranges::find_if(
		_rootModules,
		[baseTypeId_, rootModule_](const auto& entry_) {
			return entry_.baseTypeId == baseTypeId_ && entry_.module == rootModule_;
		}
	);
	_rootModules.erase(it);
}

size_t Modules::getCoreModuleCount() const noexcept {
	return _coreModules.size();
}

void Modules::addCoreModule(cref<DependencyKey> key_, mref<type_id> baseTypeId_, nmpt<CoreModule> coreModule_) {
	::hg::assertrt(
		std::ranges::find_if(
			_coreModules,
			[key_, baseTypeId_](const auto& entry_) {
				return entry_.key == key_ || entry_.baseTypeId == baseTypeId_;
			}
		) == std::ranges::end(_coreModules)
	);
	_coreModules.emplace_back(key_, std::move(baseTypeId_), std::move(coreModule_));
}

nmpt<CoreModule> Modules::getCoreModule(
	cref<decltype(std::declval<decltype(_coreModules)::value_type>().key)> key_
) const noexcept {
	const auto it = std::ranges::find_if(
		_coreModules,
		[key_](const auto& entry_) {
			return entry_.key == key_;
		}
	);
	return (it != std::ranges::end(_coreModules)) ? it->module : nullptr;
}

nmpt<CoreModule> Modules::getCoreModule(
	mref<decltype(std::declval<decltype(_coreModules)::value_type>().baseTypeId)> baseTypeId_
) const noexcept {
	const auto it = std::ranges::find_if(
		_coreModules,
		[baseTypeId_](const auto& entry_) {
			return entry_.baseTypeId == baseTypeId_;
		}
	);
	return (it != std::ranges::end(_coreModules)) ? it->module : nullptr;
}

void Modules::removeCoreModule(mref<type_id> baseTypeId_, nmpt<CoreModule> coreModule_) {

	// TODO: Validate internal dependency state, instead of force requiring non-submodule state
	::hg::assertrt(_subModules.empty());

	const auto it = std::ranges::find_if(
		_coreModules,
		[baseTypeId_, coreModule_](const auto& entry_) {
			return entry_.baseTypeId == baseTypeId_ && entry_.module == coreModule_;
		}
	);
	_coreModules.erase(it);
}

size_t Modules::getSubModuleCount() const noexcept {
	return _subModules.size();
}

cref<Deque<uptr<SubModule>>> Modules::getSubModules() const noexcept {
	return _subModules;
}

DependencyValidationResult Modules::addSubModule(mref<uptr<SubModule>> subModule_) {

	const auto where = std::ranges::find(
		_subModules,
		subModule_->moduleKey(),
		[](cref<uptr<SubModule>> module_) {
			return module_->moduleKey();
		}
	);

	if (where != _subModules.end()) {
		return DependencyValidationResult::eFailedUnique;
	}

	/**/

	for (const auto& dependency : subModule_->dependencies()) {
		if (not validateDependency(dependency)) {
			return DependencyValidationResult::eFailedOrder;
		}
	}

	/**/

	if (_subModules.empty()) {
		_subModules.push_back(std::move(subModule_));
		return DependencyValidationResult::eSuccess;
	}

	/**/

	CompactSet<ptr<SubModule>> modulePredecessors {};
	CompactSet<ptr<SubModule>> moduleSuccessores {};

	/**
	 * Resolve dependencies for the new stage
	 */
	for (const auto& dependency : subModule_->dependencies()) {

		const auto where = std::ranges::find(
			_subModules,
			dependency.key,
			[](cref<uptr<SubModule>> module_) {
				return module_->moduleKey();
			}
		);

		/**
		 * Don't prevent pushing with unsatified required dependencies
		 */
		if (where == _subModules.end()) {
			continue;
		}

		if (dependency.order == SubModuleOrder::ePredecessor) {
			moduleSuccessores.insert((*where).get());
		} else if (dependency.order == SubModuleOrder::eSuccessor) {
			modulePredecessors.insert((*where).get());
		}
	}

	/**
	 * Resolve dependencies for previous inserted stages
	 */
	for (const auto& stored : _subModules) {

		const auto& deps = stored->dependencies();

		const auto where = std::ranges::find(
			deps,
			subModule_->moduleKey(),
			[](cref<SubModuleDependency> entry_) {
				return entry_.key;
			}
		);

		if (where == deps.end()) {
			continue;
		}

		if (where->order == SubModuleOrder::ePredecessor) {
			modulePredecessors.insert(stored.get());
		} else if (where->order == SubModuleOrder::eSuccessor) {
			moduleSuccessores.insert(stored.get());
		}
	}

	/**
	 * Check for intersection of succesors and predecessors ~ dependency collision
	 */
	for (const auto& successor : moduleSuccessores) {
		if (modulePredecessors.contains(successor)) {
			return DependencyValidationResult::eFailedOrder;
		}
	}

	/**
	 * Use predecessors to find suitable spot to insert new module
	 */
	auto pos = _subModules.end();
	for (auto it = _subModules.begin(); it != _subModules.end(); ++it) {
		if (modulePredecessors.contains(it->get())) {
			pos = it;
		}
	}

	/**
	 * Insert module at suitable position
	 */
	if (pos != _subModules.end()) {
		_subModules.insert(++pos, std::move(subModule_));
	} else {
		_subModules.push_front(std::move(subModule_));
	}

	return DependencyValidationResult::eSuccess;
}

nmpt<SubModule> Modules::getSubModule(cref<DependencyKey> moduleKey_) const noexcept {
	const auto iter = std::ranges::find_if(
		_subModules,
		[moduleKey_](const uptr<SubModule>& module_) {
			return module_->moduleKey() == moduleKey_;
		}
	);

	if (iter != _subModules.end()) {
		return (*iter).get();
	}

	return nullptr;
}

bool Modules::removeSubModule(nmpt<SubModule> subModule_) {

	const auto rem = std::ranges::remove(
		_subModules,
		subModule_.get(),
		[](cref<uptr<SubModule>> module_) {
			return module_.get();
		}
	);

	_subModules.erase(rem.begin(), rem.end());

	return false;
}

DependencyValidationResult Modules::validate() const noexcept {

	/**
	 * Collect all required modules dependencies
	 */
	CompactSet<DependencyKey> required {};
	for (const auto& subModule : _subModules) {
		for (const auto& dep : subModule->dependencies()) {
			if (dep.required) {
				required.insert(dep.key);
			}
		}
	}

	/**
	 * Validate every module is present
	 */
	for (const auto& subModule : _subModules) {
		required.erase(subModule->moduleKey());
	}

	for (const auto& coreModule : _coreModules) {
		required.erase(coreModule.key);
	}

	return required.empty() ? DependencyValidationResult::eSuccess : DependencyValidationResult::eFailedRequired;
}
