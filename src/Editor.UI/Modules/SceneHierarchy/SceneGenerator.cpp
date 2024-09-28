#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Widget/Text.hpp>
#include <Engine.Reflow/Widget/Widget.hpp>
#include <Heliogrim/Actor/Actor.hpp>
#include <Heliogrim/Component/HierarchyComponent.hpp>

#include "HierarchyGenerator.hpp"
#include "SceneViewEntry.hpp"
#include "Editor.UI/Theme/Theme.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

template <>
sptr<Widget> HierarchyGenerator<sptr<SceneViewEntry>>::operator()(
	cref<sptr<SceneViewEntry>> source_
) const {

	const auto theme = Theme::get();

	/**/

	if (source_->type() == SceneViewEntryType::eActor) {

		const ptr<Actor> actor { source_->target<Actor>() };

		auto txt = make_sptr<Text>();
		theme->applyLabel(txt);

		const auto actorGuid = actor->guid();
		txt->setText(
			std::format(
				R"(Actor << {}-{}-{}-{} >>)",
				actorGuid.pre,
				actorGuid.c0,
				actorGuid.c1,
				actorGuid.post
			)
		);

		return txt;
	}

	if (source_->type() == SceneViewEntryType::eComponent) {

		const ptr<HierarchyComponent> comp { source_->target<HierarchyComponent>() };

		auto txt = make_sptr<Text>();
		theme->applyLabel(txt);

		u64 depth { 0uLL };
		auto parent { comp };
		while ((parent = parent->getParentComponent())) {
			++depth;
		}

		if (depth <= 0uLL) {
			txt->setText(std::format(R"(Root Component << {} >>)", comp->getTypeId().data));
		} else {
			txt->setText(std::format(R"(Component [D{}] << {} >>)", depth, comp->getTypeId().data));
		}

		return txt;
	}

	return nullptr;
}
