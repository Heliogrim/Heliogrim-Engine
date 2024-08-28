#include <Engine.Common/Make.hpp>
#include <Heliogrim/Actor/Actor.hpp>
#include <Heliogrim/Component/HierarchyComponent.hpp>

#include "HierarchyResolver.hpp"
#include "SceneViewEntry.hpp"

using namespace hg::editor::ui;
using namespace hg;

template <>
void HierarchyResolver<sptr<SceneViewEntry>>::operator()(
	cref<sptr<SceneViewEntry>> source_,
	ref<Vector<resolve_type>> resolved_
) const {

	if (source_->type() == SceneViewEntryType::eEmpty) {
		return;
	}

	if (source_->type() == SceneViewEntryType::eActor) {

		const ptr<Actor> actor { source_->target<Actor>() };

		const auto root { actor->getRootComponent() };
		if (not root) {
			return;
		}

		auto next { make_sptr<SceneViewEntry>() };
		next->storeTarget<HierarchyComponent>(root);

		resolved_.push_back(std::move(next));
		return;
	}

	if (source_->type() == SceneViewEntryType::eComponent) {

		const ptr<HierarchyComponent> comp { source_->target<HierarchyComponent>() };
		const ptr<Actor> actor { comp->getOwner() };

		if (not actor) {
			return;
		}

		const auto selection {
			actor->selectComponents(
				[comp](const ptr<HierarchyComponent> component_) {
					return component_->getParentComponent() == comp;
				}
			)
		};

		for (const auto& entry : selection) {

			auto next { make_sptr<SceneViewEntry>() };
			next->storeTarget<HierarchyComponent>(entry);

			resolved_.push_back(std::move(next));
		}

		// return;
	}

}
