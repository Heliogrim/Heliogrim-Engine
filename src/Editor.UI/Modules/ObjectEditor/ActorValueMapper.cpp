#include <Heliogrim/Actor.hpp>
#include <Heliogrim/ActorComponent.hpp>
#include <Engine.Reflow/Widget/Input/InputText.hpp>

#include "ObjectValueMapper.hpp"
#include "../../Widget/Input/InputVec.hpp"
#include "Editor.UI/Theme/Theme.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

template <>
void ObjectValueMapper<Actor>::build(cref<sptr<VerticalPanel>> parent_) {

    const auto* const theme = Theme::get();

    auto actorGuid = make_sptr<Text>();
    theme->applyText(actorGuid);
    actorGuid->setText(R"(<<Actor Guid>>)");

    auto pos = make_sptr<InputVec3>();
    auto rot = make_sptr<InputVec3>();
    auto scale = make_sptr<InputVec3>();

    pos->setLabel(0, "X");
    pos->setLabel(1, "Y");
    pos->setLabel(2, "Z");

    rot->setLabel(0, "P");
    rot->setLabel(1, "Y");
    rot->setLabel(2, "R");

    scale->setLabel(0, "X");
    scale->setLabel(1, "Y");
    scale->setLabel(2, "Z");

    /**/

    parent_->addChild(actorGuid);
    parent_->addChild(pos);
    parent_->addChild(rot);
    parent_->addChild(scale);

}

template <>
void ObjectValueMapper<Actor>::update(cref<sptr<VerticalPanel>> parent_, const ptr<void> obj_) {

    const auto& actor { *static_cast<ptr<Actor>>(obj_) };
    const auto& mat { actor.getWorldTransform() };
    const auto& children { *parent_->children() };

    const auto actorGuid = actor.guid();
    _STD static_pointer_cast<Text, Widget>(children.front())->setText(
        _STD format(R"({}-{}-{}-{})", actorGuid.pre, actorGuid.c0, actorGuid.c1, actorGuid.post)
    );

    _STD static_pointer_cast<InputVec3, Widget>(children[1])->setValue(mat.location().fvec3());
    _STD static_pointer_cast<InputVec3, Widget>(children[2])->setValue(mat.rotator().euler());
    _STD static_pointer_cast<InputVec3, Widget>(children[3])->setValue(mat.scale());

    /**/

    _STD static_pointer_cast<InputVec3, Widget>(children[1])->_callback = [actor = &actor](math::vec3 value_) {
        const_cast<ref<Transform>>(actor->getRootComponent()->getWorldTransform()).setLocation(
            math::Location(_STD move(value_))
        );
    };

    _STD static_pointer_cast<InputVec3, Widget>(children[2])->_callback = [actor = &actor](math::vec3 value_) {
        const_cast<ref<Transform>>(actor->getRootComponent()->getWorldTransform()).setRotator(
            math::Rotator::fromEuler(_STD move(value_))
        );
    };

    _STD static_pointer_cast<InputVec3, Widget>(children[3])->_callback = [actor = &actor](math::vec3 value_) {
        const_cast<ref<Transform>>(actor->getRootComponent()->getWorldTransform()).setScale(_STD move(value_));
    };
}

template <>
void ObjectValueMapper<Actor>::cleanup(cref<sptr<VerticalPanel>> parent_) {
    parent_->clearChildren();
}
