#include "Modules.hpp"

#include <algorithm>

#include "CoreDependencies.hpp"
#include "SubModule.hpp"
#include "SubModuleDependency.hpp"

using namespace hg::engine::core;
using namespace hg;

Modules::Modules() :
	_subModules(),
	_coreModules(
		{
			PlatformDepKey,
			ResourcesDepKey,
			SchedulerDepKey,
			/**/
			AssetsDepKey,
			AudioDepKey,
			GraphicsDepKey,
			InputDepKey,
			NetworkDepKey,
			PhysicsDepKey
		}
	) {}

Modules::~Modules() = default;

bool Modules::validateDependency(cref<SubModuleDependency> dependency_) const noexcept {

	if (not _coreModules.contains(dependency_.key)) {
		return true;
	}

	return dependency_.order != SubModuleOrder::ePredecessor;
}

size_t Modules::getCoreModuleCount() const noexcept {
	return _coreModules.size();
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
		required.erase(coreModule);
	}

	return required.empty() ? DependencyValidationResult::eSuccess : DependencyValidationResult::eFailedRequired;
}
