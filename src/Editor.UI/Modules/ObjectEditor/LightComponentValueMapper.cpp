#include <Engine.Reflow/Widget/Text.hpp>
#include <Engine.Reflow/Widget/Input/InputFloat.hpp>
#include <Engine.Reflow/Widget/Input/InputIntegral.hpp>
#include <Engine.Assets/Types/Geometry/StaticGeometry.hpp>
#include <Heliogrim/Components/DirectionalLightComponent.hpp>
#include <Heliogrim/Components/PointLightComponent.hpp>

#include "Engine.GFX.Glow.UI/TestUI.hpp"

#include "ObjectValueMapper.hpp"
#include "../../Color/Dark.hpp"
#include "../../Widget/Input/InputVec.hpp"
#include "../../Widget/Collapse.hpp"
#include "Editor.UI/Theme/Theme.hpp"
#include "Heliogrim/Heliogrim.hpp"
#include "Engine.Assets/Types/Material/GfxMaterial.hpp"
#include "Engine.Assets.System/IAssetRegistry.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

#pragma region Point Light Component

template <>
void ObjectValueMapper<PointLightComponent>::build(cref<sptr<engine::reflow::VerticalPanel>> parent_) {

    const auto* const theme = Theme::get();

    auto title = make_sptr<Text>();
    theme->applyText(title);
    title->setText(R"(<<Point Light Component>>)");

    parent_->addChild(title);

    /**/
    auto transform { make_sptr<Collapse>() };
    transform->setup();
    transform->getHeader()->setTitle("Transform");

    parent_->addChild(transform);

    {
        auto wrapper = make_sptr<VerticalPanel>();

        wrapper->attr.minWidth.setValue({ ReflowUnitType::eRelative, 1.F });
        wrapper->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
        wrapper->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
        wrapper->attr.minHeight.setValue({ ReflowUnitType::eAuto, 0.F });
        wrapper->attr.height.setValue({ ReflowUnitType::eAuto, 0.F });
        wrapper->attr.colGap.setValue(4.F);
        wrapper->attr.justify.setValue(ReflowSpacing::eStart);
        wrapper->attr.flexGrow.setValue(1);
        wrapper->attr.flexShrink.setValue(1);
        wrapper->attr.style.setValue(PanelStyle { .backgroundColor = engine::color { 255.F, 20.F, 20.F, 255.F } });

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

        wrapper->addChild(pos);
        wrapper->addChild(rot);
        wrapper->addChild(scale);

        transform->setContent(wrapper);
    }

    /**/
    auto light { make_sptr<Collapse>() };
    light->setup();
    light->getHeader()->setTitle("Point Light");

    parent_->addChild(light);

    {
        auto wrapper = make_sptr<VerticalPanel>();

        wrapper->attr.minWidth.setValue({ ReflowUnitType::eRelative, 1.F });
        wrapper->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
        wrapper->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
        wrapper->attr.minHeight.setValue({ ReflowUnitType::eAuto, 0.F });
        wrapper->attr.height.setValue({ ReflowUnitType::eAuto, 0.F });
        wrapper->attr.colGap.setValue(4.F);
        wrapper->attr.justify.setValue(ReflowSpacing::eStart);
        wrapper->attr.flexGrow.setValue(1);
        wrapper->attr.flexShrink.setValue(1);
        wrapper->attr.style.setValue(PanelStyle { .backgroundColor = engine::color { 255.F, 20.F, 20.F, 255.F } });

        auto luminance = make_sptr<InputVec3>();
        auto distance = make_sptr<InputFloat>();

        luminance->setLabel(0, "R");
        luminance->setLabel(1, "G");
        luminance->setLabel(2, "B");

        {
            auto& style = distance->_wrapper->attr;

            style.minWidth.setValue({ engine::reflow::ReflowUnitType::eRelative, 1.F });
            style.width.setValue({ engine::reflow::ReflowUnitType::eRelative, 1.F });
            style.maxWidth.setValue({ engine::reflow::ReflowUnitType::eRelative, 1.F });

            style.minHeight.setValue({ engine::reflow::ReflowUnitType::eAbsolute, 20.F });
            style.height.setValue({ engine::reflow::ReflowUnitType::eAbsolute, 20.F });
            style.maxHeight.setValue({ engine::reflow::ReflowUnitType::eAbsolute, 20.F });

            style.padding.setValue(engine::reflow::Padding { 4.F, 2.F });

            style.style.setValue(
                engine::reflow::PanelStyle { .backgroundColor = engine::color { 40.F, 44.F, 52.F, 255.F } }
            );

            theme->applyText(distance->_text);
        }

        wrapper->addChild(luminance);
        wrapper->addChild(distance);

        light->setContent(wrapper);
    }
}

template <>
void ObjectValueMapper<PointLightComponent>::update(
    cref<sptr<engine::reflow::VerticalPanel>> parent_,
    const ptr<void> obj_
) {

    const auto& sgc { *static_cast<ptr<PointLightComponent>>(obj_) };
    const auto& mat { sgc.getWorldTransform() };
    const auto& children { *parent_->children() };

    auto* const transform { static_cast<ptr<Collapse>>(children[1].get()) };

    {
        const auto wrapper = std::static_pointer_cast<VerticalPanel, Widget>(transform->getContent());

        std::static_pointer_cast<InputVec3, Widget>(wrapper->children()->at(0))->setValue(mat.location().fvec3());
        std::static_pointer_cast<InputVec3, Widget>(wrapper->children()->at(1))->setValue(mat.rotator().euler());
        std::static_pointer_cast<InputVec3, Widget>(wrapper->children()->at(2))->setValue(mat.scale());

        std::static_pointer_cast<InputVec3, Widget>(wrapper->children()->at(0))->_callback = [sgc = &sgc](
            math::vec3 value_
        ) {
                const_cast<ref<Transform>>(sgc->getWorldTransform()).setLocation(math::Location(std::move(value_)));
            };

        std::static_pointer_cast<InputVec3, Widget>(wrapper->children()->at(1))->_callback = [sgc = &sgc](
            math::vec3 value_
        ) {
                const_cast<ref<Transform>>(sgc->getWorldTransform()).setRotator(
                    math::Rotator::fromEuler(std::move(value_))
                );
            };

        std::static_pointer_cast<InputVec3, Widget>(wrapper->children()->at(2))->_callback = [sgc = &sgc
            ](math::vec3 value_) {

                const_cast<ref<Transform>>(sgc->getWorldTransform()).setScale(std::move(value_));
            };
    }

    /**/

    auto* const lightCollapse { static_cast<ptr<Collapse>>(children[2].get()) };

    {
        const auto wrapper = std::static_pointer_cast<VerticalPanel, Widget>(lightCollapse->getContent());

        auto* const luminance = std::static_pointer_cast<InputVec3, Widget>(wrapper->children()->at(0)).get();
        auto* const limit = std::static_pointer_cast<InputFloat, Widget>(wrapper->children()->at(1)).get();

        luminance->setValue(sgc._luminance);
        limit->setValue(sgc._distanceLimit);

        luminance->_callback = [sgc = &sgc](math::fvec3 value_) {
            const_cast<ref<math::fvec3>>(sgc->_luminance) = value_;
        };
        limit->_callback = [sgc = &sgc](float value_) {
            const_cast<ref<float>>(sgc->_distanceLimit) = value_;
        };
    }
}

template <>
void ObjectValueMapper<PointLightComponent>::cleanup(cref<sptr<engine::reflow::VerticalPanel>> parent_) {
    parent_->clearChildren();
}

#pragma endregion Point Light Component

#pragma region Point Light Component

template <>
void ObjectValueMapper<DirectionalLightComponent>::build(cref<sptr<engine::reflow::VerticalPanel>> parent_) {

    const auto* const theme = Theme::get();

    auto title = make_sptr<Text>();
    theme->applyText(title);
    title->setText(R"(<<Directional Light Component>>)");

    parent_->addChild(title);

    /**/
    auto transform { make_sptr<Collapse>() };
    transform->setup();
    transform->getHeader()->setTitle("Transform");

    parent_->addChild(transform);

    {
        auto wrapper = make_sptr<VerticalPanel>();

        wrapper->attr.minWidth.setValue({ ReflowUnitType::eRelative, 1.F });
        wrapper->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
        wrapper->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
        wrapper->attr.minHeight.setValue({ ReflowUnitType::eAuto, 0.F });
        wrapper->attr.height.setValue({ ReflowUnitType::eAuto, 0.F });
        wrapper->attr.colGap.setValue(4.F);
        wrapper->attr.justify.setValue(ReflowSpacing::eStart);
        wrapper->attr.flexGrow.setValue(1);
        wrapper->attr.flexShrink.setValue(1);
        wrapper->attr.style.setValue(PanelStyle { .backgroundColor = engine::color { 255.F, 20.F, 20.F, 255.F } });

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

        wrapper->addChild(pos);
        wrapper->addChild(rot);
        wrapper->addChild(scale);

        transform->setContent(wrapper);
    }

    /**/
    auto light { make_sptr<Collapse>() };
    light->setup();
    light->getHeader()->setTitle("Directional Light");

    parent_->addChild(light);

    {
        auto wrapper = make_sptr<VerticalPanel>();

        wrapper->attr.minWidth.setValue({ ReflowUnitType::eRelative, 1.F });
        wrapper->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
        wrapper->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
        wrapper->attr.minHeight.setValue({ ReflowUnitType::eAuto, 0.F });
        wrapper->attr.height.setValue({ ReflowUnitType::eAuto, 0.F });
        wrapper->attr.colGap.setValue(4.F);
        wrapper->attr.justify.setValue(ReflowSpacing::eStart);
        wrapper->attr.flexGrow.setValue(1);
        wrapper->attr.flexShrink.setValue(1);
        wrapper->attr.style.setValue(PanelStyle { .backgroundColor = engine::color { 255.F, 20.F, 20.F, 255.F } });

        auto luminance = make_sptr<InputVec3>();
        auto direction = make_sptr<InputVec3>();

        luminance->setLabel(0, "R");
        luminance->setLabel(1, "G");
        luminance->setLabel(2, "B");

        direction->setLabel(0, "X");
        direction->setLabel(1, "Y");
        direction->setLabel(2, "Z");

        wrapper->addChild(luminance);
        wrapper->addChild(direction);

        light->setContent(wrapper);
    }
}

template <>
void ObjectValueMapper<DirectionalLightComponent>::update(
    cref<sptr<engine::reflow::VerticalPanel>> parent_,
    const ptr<void> obj_
) {

    const auto& dlc { *static_cast<ptr<DirectionalLightComponent>>(obj_) };
    const auto& mat { dlc.getWorldTransform() };
    const auto& children { *parent_->children() };

    auto* const transform { static_cast<ptr<Collapse>>(children[1].get()) };

    {
        const auto wrapper = std::static_pointer_cast<VerticalPanel, Widget>(transform->getContent());

        std::static_pointer_cast<InputVec3, Widget>(wrapper->children()->at(0))->setValue(mat.location().fvec3());
        std::static_pointer_cast<InputVec3, Widget>(wrapper->children()->at(1))->setValue(mat.rotator().euler());
        std::static_pointer_cast<InputVec3, Widget>(wrapper->children()->at(2))->setValue(mat.scale());

        std::static_pointer_cast<InputVec3, Widget>(wrapper->children()->at(0))->_callback = [sgc = &dlc](
            math::vec3 value_
        ) {
                const_cast<ref<Transform>>(sgc->getWorldTransform()).setLocation(math::Location(std::move(value_)));
            };

        std::static_pointer_cast<InputVec3, Widget>(wrapper->children()->at(1))->_callback = [sgc = &dlc](
            math::vec3 value_
        ) {
                const_cast<ref<Transform>>(sgc->getWorldTransform()).setRotator(
                    math::Rotator::fromEuler(std::move(value_))
                );
            };

        std::static_pointer_cast<InputVec3, Widget>(wrapper->children()->at(2))->_callback = [sgc = &dlc
            ](math::vec3 value_) {

                const_cast<ref<Transform>>(sgc->getWorldTransform()).setScale(std::move(value_));
            };
    }

    /**/

    auto* const lightCollapse { static_cast<ptr<Collapse>>(children[2].get()) };

    {
        const auto wrapper = std::static_pointer_cast<VerticalPanel, Widget>(lightCollapse->getContent());

        auto* const luminance = std::static_pointer_cast<InputVec3, Widget>(wrapper->children()->at(0)).get();
        auto* const direction = std::static_pointer_cast<InputVec3, Widget>(wrapper->children()->at(1)).get();

        luminance->setValue(dlc._luminance);

        const auto euler = dlc._direction.euler();
        const auto deg = math::fvec3 { glm::degrees(euler.x), glm::degrees(euler.y), glm::degrees(euler.z) };
        direction->setValue(deg);

        luminance->_callback = [sgc = &dlc](math::fvec3 value_) {
            const_cast<ref<math::fvec3>>(sgc->_luminance) = value_;
        };
        direction->_callback = [sgc = &dlc](math::fvec3 value_) {
            const auto rad = math::fvec3 { glm::radians(value_.x), glm::radians(value_.y), glm::radians(value_.z) };
            const_cast<ref<math::Rotator>>(sgc->_direction) = math::Rotator { math::quaternion::euler(rad) };
        };
    }
}

template <>
void ObjectValueMapper<DirectionalLightComponent>::cleanup(cref<sptr<engine::reflow::VerticalPanel>> parent_) {
    parent_->clearChildren();
}

#pragma endregion Point Light Component

#pragma region Spot Light Component
#pragma endregion Spot Light Component
