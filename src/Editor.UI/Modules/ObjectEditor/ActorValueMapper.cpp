#include <Engine.Reflow/Widget/Input/InputText.hpp>
#include <Heliogrim/Actor/Actor.hpp>
#include <Heliogrim/Component/HierarchyComponent.hpp>

#include "ObjectValueMapper.hpp"
#include "../../Widget/Input/InputVec.hpp"
#include "Editor.UI/Theme/Theme.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

template <>
void ObjectValueMapper<Actor>::build(cref<sptr<VerticalPanel>> parent_) {

	const auto theme = Theme::get();

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
	const auto& mat { actor.getUniverseTransform() };
	const auto& children { *parent_->children() };

	const auto actorGuid = actor.guid();
	std::static_pointer_cast<Text, Widget>(children.front())->setText(
		std::format(R"({}-{}-{}-{})", actorGuid.pre, actorGuid.c0, actorGuid.c1, actorGuid.post)
	);

	std::static_pointer_cast<InputVec3, Widget>(children[1])->setValue(mat.location().into());
	std::static_pointer_cast<InputVec3, Widget>(children[2])->setValue(mat.rotator().euler());
	std::static_pointer_cast<InputVec3, Widget>(children[3])->setValue(mat.scale());

	/**/

	std::static_pointer_cast<InputVec3, Widget>(children[1])->_callback = [actor = &actor](math::vec3 value_) {
		const_cast<ref<Transform>>(actor->getRootComponent()->getUniverseTransform()).setLocation(
			math::Location(std::move(value_))
		);
	};

	std::static_pointer_cast<InputVec3, Widget>(children[2])->_callback = [actor = &actor](math::vec3 value_) {
		const_cast<ref<Transform>>(actor->getRootComponent()->getUniverseTransform()).setRotator(
			math::Rotator::fromEuler(std::move(value_))
		);
	};

	std::static_pointer_cast<InputVec3, Widget>(children[3])->_callback = [actor = &actor](math::vec3 value_) {
		const_cast<ref<Transform>>(actor->getRootComponent()->getUniverseTransform()).setScale(std::move(value_));
	};
}

template <>
void ObjectValueMapper<Actor>::cleanup(cref<sptr<VerticalPanel>> parent_) {
	parent_->clearChildren();
}
