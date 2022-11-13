#include <Ember/Actor.hpp>
#include <Engine.Reflow/Widget/Widget.hpp>
#include <Engine.Reflow/Widget/Text.hpp>
#include <Engine.Reflow/Style/BoundStyleSheet.hpp>
#include <Engine.Reflow/Style/StyleCondition.hpp>

#include "HierarchyGenerator.hpp"
#include "SceneViewEntry.hpp"
#include "../../Style/Style.hpp"

using namespace ember::editor::ui;
using namespace ember::engine::reflow;
using namespace ember;

template <>
sptr<Widget> HierarchyGenerator<sptr<SceneViewEntry>>::operator()(
    cref<sptr<SceneViewEntry>> source_
) const {

    if (source_->type() == SceneViewEntryType::eActor) {

        const ptr<Actor> actor { source_->target<Actor>() };

        auto txt { make_sptr<Text>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::TitleSmallKey))) };
        txt->style().color.unset();
        txt->style().pushStyle({
            AssocKey<string>::from(R"(TreeItem::Selected)"),
            style::isNever,
            Style::get()->getStyleSheet(Style::TitleRaisedKey)
        });

        txt->setText(_STD format(R"(Actor << {} >>)", actor->guid().as_uint64()));

        return txt;
    }

    if (source_->type() == SceneViewEntryType::eComponent) {

        const ptr<ActorComponent> comp { source_->target<ActorComponent>() };

        auto txt { make_sptr<Text>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::TitleSmallKey))) };
        txt->style().color.unset();
        txt->style().pushStyle({
            AssocKey<string>::from(R"(TreeItem::Selected)"),
            style::isNever,
            Style::get()->getStyleSheet(Style::TitleRaisedKey)
        });

        u64 depth { 0ui64 };
        auto parent { comp };
        while ((parent = parent->getParentComponent())) {
            ++depth;
        }

        if (depth <= 0ui64) {
            txt->setText(_STD format(R"(Root Component << {} >>)", comp->getTypeId().data));
        } else {
            txt->setText(_STD format(R"(Component [D{}] << {} >>)", depth, comp->getTypeId().data));
        }

        return txt;
    }

    return nullptr;
}
