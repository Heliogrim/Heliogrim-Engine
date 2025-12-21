#include "SceneHierarchyService.hpp"

#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Heliogrim/Actor/Actor.hpp>

#include "SceneHierarchyEntry.hpp"

using namespace hg::editor::ui::service;
using namespace hg;

SceneHierarchyService::SceneHierarchyService() = default;

SceneHierarchyService::~SceneHierarchyService() = default;

void SceneHierarchyService::fetchAll(ref<const SceneHierarchyEntry> base_, ref<Vector<SceneHierarchyEntry>> entries_) {

	if (base_.type() == SceneHierarchyEntryType::eActor) {

		auto root = base_.value.as<nmpt<Actor>>()->getRootComponent();
		if (root.is_null()) {
			return;
		}

		const auto next = SceneHierarchyEntry { std::addressof(root.value()) };
		entries_.emplace_back(clone(next));

		return fetchAll(next, entries_);
	}

	if (base_.type() == SceneHierarchyEntryType::eComponent) {

		const auto parent = base_.value.as<nmpt<HierarchyComponent>>();
		const auto actor = parent->getOwner();

		if (actor == nullptr) {
			return;
		}

		const auto selection = actor->selectComponents(
			[parent](const ptr<HierarchyComponent> component_) {
				return component_->getParentComponent() == parent.get();
			}
		);

		for (const auto& selected : selection) {
			const auto next = SceneHierarchyEntry { selected };
			entries_.emplace_back(clone(next));

			fetchAll(next, entries_);
		}

	}

}

void SceneHierarchyService::fetchLayer(ref<const SceneHierarchyEntry> parent_, ref<Vector<SceneHierarchyEntry>> children_) {

	if (parent_.type() == SceneHierarchyEntryType::eActor) {

		auto root = parent_.value.as<nmpt<Actor>>()->getRootComponent();
		if (root.is_null()) {
			return;
		}

		const auto next = SceneHierarchyEntry { std::addressof(root.value()) };
		children_.emplace_back(clone(next));
		return;
	}

	if (parent_.type() == SceneHierarchyEntryType::eComponent) {

		const auto parent = parent_.value.as<nmpt<HierarchyComponent>>();
		const auto actor = parent->getOwner();

		if (actor == nullptr) {
			return;
		}

		const auto selection = actor->selectComponents(
			[parent](const ptr<HierarchyComponent> component_) {
				return component_->getParentComponent() == parent.get();
			}
		);

		for (const auto& selected : selection) {
			const auto next = SceneHierarchyEntry { selected };
			children_.emplace_back(clone(next));
		}
	}

}
