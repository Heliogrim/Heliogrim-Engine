#include <Engine.Common/Make.hpp>
#include <Heliogrim/Actor.hpp>
#include <Heliogrim/ActorComponent.hpp>
#include <Engine.Reflow/Widget/Widget.hpp>
#include <Engine.Reflow/Widget/Text.hpp>

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

    const auto* const theme = Theme::get();

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

        const ptr<ActorComponent> comp { source_->target<ActorComponent>() };

        auto txt = make_sptr<Text>();
        theme->applyLabel(txt);

        u64 depth { 0ui64 };
        auto parent { comp };
        while ((parent = parent->getParentComponent())) {
            ++depth;
        }

        if (depth <= 0ui64) {
            txt->setText(std::format(R"(Root Component << {} >>)", comp->getTypeId().data));
        } else {
            txt->setText(std::format(R"(Component [D{}] << {} >>)", depth, comp->getTypeId().data));
        }

        return txt;
    }

    return nullptr;
}
