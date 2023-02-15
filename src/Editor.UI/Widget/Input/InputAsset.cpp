#include "InputAsset.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Style/BoundStyleSheet.hpp>
#include <Engine.Reflow/Style/StyleSheet.hpp>
#include <Engine.Reflow/Style/StyleCondition.hpp>
#include "../../Style/Style.hpp"
#include "../../Color/Dark.hpp"
#include "../../Helper/AssetBrowserHelper.hpp"

#include "Engine.GFX.Glow.UI/TestUI.hpp"
#include "Engine.GFX.Loader/Texture/TextureResource.hpp"
#include "Engine.Resource/LoaderManager.hpp"
#include "Engine.Resource/ResourceManager.hpp"
#include <Ember/AssetDatabase.hpp>
#include <Ember/AssetDatabaseResult.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX.Loader/Texture/Traits.hpp>

#include "Ember/Ember.hpp"

using namespace ember::editor::ui;
using namespace ember::engine::reflow;
using namespace ember;

[[nodiscard]] static sptr<BoundStyleSheet> makeInputBoxStyle() {

    auto style {
        BoundStyleSheet::make(
            StyleSheet {
                .minWidth { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .width { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .maxWidth { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .minHeight { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
                .height { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
                .maxHeight { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
                .wrap { true, ReflowWrap::eNoWrap },
                .padding { true, Padding { 4.F, 2.F } },
                .margin { true, Margin { 0.F } },
                .reflowShrink { true, 1.F },
                .reflowGrow { true, 0.F },
                .borderRadius = { true, BorderRadius { 4.F } },
                .color { false, color::Dark::backgroundInnerField }
            }
        )

    };

    style->pushStyle(
        {
            Style::key_type::from("InputType::Focused"),
            style::isFocused,
            make_sptr<StyleSheet>(
                StyleSheet {
                    .color { true, color::Dark::backgroundInnerFieldDarken }
                }
            )
        }
    );

    return style;

}

[[nodiscard]] static sptr<BoundStyleSheet> makeInputTextStyle() {
    auto* font { getDefaultFont() };

    auto style {
        BoundStyleSheet::make(
            StyleSheet {
                .minWidth { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .width { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .maxWidth { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .minHeight { true, ReflowUnit { ReflowUnitType::eAbsolute, 16.F } },
                .height { true, ReflowUnit { ReflowUnitType::eAbsolute, 16.F } },
                .maxHeight { true, ReflowUnit { ReflowUnitType::eAbsolute, 16.F } },
                .color { false, color::Dark::grey },
                .font { true, font },
                .fontSize { true, 16.F },
                .textAlign { true, TextAlign::eLeftMiddle },
                .textEllipse { true, 1ui32 }
            }
        )
    };

    style->pushStyle(
        {
            Style::key_type::from("InputType::Focused"),
            style::isNever,
            make_sptr<StyleSheet>(
                StyleSheet {
                    .color { true, color::Dark::white }
                }
            )
        }
    );

    return style;
}

InputAsset::InputAsset() :
    Input(),
    _value(invalid_asset_guid) {
    setup();
}

void InputAsset::setup() {

    _content = make_sptr<VBox>(
        BoundStyleSheet::make(
            StyleSheet {
                .minWidth { true, ReflowUnit { ReflowUnitType::eAbsolute, 72.F + 4.F + 16.F } },
                .width { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                .minHeight { true, ReflowUnit { ReflowUnitType::eAbsolute, 72.F + 4.F + 20.F } },
                .maxHeight { true, ReflowUnit { ReflowUnitType::eAbsolute, 72.F + 4.F + 20.F } },
                .wrap { true, ReflowWrap::eNoWrap },
                .colGap { true, ReflowUnit { ReflowUnitType::eAbsolute, 4.F } },
                .color { true, color::Dark::backgroundDefault },
            }
        )
    );

    _children.push_back(_content);

    /**/

    auto upper {
        make_sptr<HBox>(
            BoundStyleSheet::make(
                StyleSheet {
                    .minWidth { true, ReflowUnit { ReflowUnitType::eAbsolute, 72.F + 4.F + 16.F } },
                    .width { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                    //.maxWidth { true, ReflowUnit { ReflowUnitType::eAbsolute, 72.F + 4.F + 16.F } },
                    .minHeight { true, ReflowUnit { ReflowUnitType::eAbsolute, 72.F } },
                    .maxHeight { true, ReflowUnit { ReflowUnitType::eAbsolute, 72.F } },
                    .wrap { true, ReflowWrap::eNoWrap },
                    .rowGap { true, ReflowUnit { ReflowUnitType::eAbsolute, 4.F } },
                    .reflowSpacing { true, ReflowSpacing::eSpaceAround },
                    .color { true, color::Dark::backgroundDefault },
                }
            )
        )
    };
    _content->addChild(upper);

    auto previewBox { make_sptr<VBox>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::Icon72Key))) };
    previewBox->style()->color = color::Dark::backgroundInnerField;
    previewBox->style()->borderRadius = BorderRadius { 4.F };
    upper->addChild(previewBox);

    _preview = make_sptr<Image>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::Icon72Key)));
    auto* previewAsset = AssetBrowserHelper::get()->getItemIconByAssetType(asset_type_id {});

    auto iconRes = engine::Engine::getEngine()->getResources()->loader().loadImmediately<engine::assets::Texture,
        engine::gfx::TextureResource>(_STD move(previewAsset));

    auto previewGuard = iconRes->acquire(engine::resource::ResourceUsageFlag::eRead);
    auto* view = previewGuard->as<engine::gfx::VirtualTextureView>();

    _preview->setImage(make_sptr<engine::gfx::ProxyTexture<non_owning_rptr>>(_STD move(view)), iconRes.get());
    previewBox->addChild(_preview);

    auto actions {
        make_sptr<VBox>(
            BoundStyleSheet::make(
                StyleSheet {
                    .minWidth { true, ReflowUnit { ReflowUnitType::eAbsolute, 16.F } },
                    .maxWidth { true, ReflowUnit { ReflowUnitType::eAbsolute, 16.F } },
                    .minHeight { true, ReflowUnit { ReflowUnitType::eAbsolute, 72.F } },
                    .maxHeight { true, ReflowUnit { ReflowUnitType::eAbsolute, 72.F } },
                    .wrap { true, ReflowWrap::eNoWrap },
                    .colGap { true, ReflowUnit { ReflowUnitType::eAbsolute, 4.F } },
                    .reflowSpacing { true, ReflowSpacing::eStart },
                    .color { true, color::Dark::backgroundDefault },
                }
            )
        )
    };
    upper->addChild(actions);

    /**/

    _search = make_sptr<Button>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::ButtonKey)));
    _search->style()->color = color::Dark::backgroundInnerField;
    actions->addChild(_search);

    auto txt { make_sptr<Text>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::TitleSmallKey))) };
    txt->setText("S");
    _search->addChild(txt);

    _reset = make_sptr<Button>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::ButtonKey)));
    _reset->style()->color = color::Dark::backgroundInnerField;
    actions->addChild(_reset);

    txt = make_sptr<Text>(BoundStyleSheet::make(Style::get()->getStyleSheet(Style::TitleSmallKey)));
    txt->setText("R");
    _reset->addChild(txt);

    /**/

    auto lower {
        make_sptr<HBox>(
            BoundStyleSheet::make(
                StyleSheet {
                    .minWidth { true, ReflowUnit { ReflowUnitType::eAbsolute, 72.F + 4.F + 16.F } },
                    .width { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                    .maxWidth { true, ReflowUnit { ReflowUnitType::eRelative, 1.F } },
                    .minHeight { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
                    .maxHeight { true, ReflowUnit { ReflowUnitType::eAbsolute, 20.F } },
                    .wrap { true, ReflowWrap::eNoWrap },
                    .color { true, color::Dark::backgroundDefault },
                }
            )
        )
    };
    _content->addChild(lower);

    _input = make_sptr<InputText>(makeInputBoxStyle(), makeInputTextStyle());
    lower->addChild(_input);
}

string InputAsset::getTag() const noexcept {
    return _STD format(R"(InputAsset <{:#x}>)", reinterpret_cast<u64>(this));
}

engine::reflow::EventResponse InputAsset::onDrop(cref<engine::reflow::DragDropEvent> event_) {
    return Input<asset_guid>::onDrop(event_);
}

engine::reflow::EventResponse InputAsset::onDragOver(cref<engine::reflow::DragDropEvent> event_) {
    return Input<asset_guid>::onDragOver(event_);
}

const ptr<const engine::reflow::Children> InputAsset::children() const {
    return &_children;
}

void InputAsset::render(const ptr<engine::reflow::ReflowCommandBuffer> cmd_) {
    _content->render(cmd_);
}

void InputAsset::flow(
    cref<engine::reflow::FlowContext> ctx_,
    cref<math::vec2> space_,
    cref<math::vec2> limit_,
    ref<engine::reflow::StyleKeyStack> styleStack_
) {
    _content->flow(ctx_, space_, limit_, styleStack_);
}

void InputAsset::shift(cref<engine::reflow::FlowContext> ctx_, cref<math::vec2> offset_) {
    _content->shift(ctx_, offset_);
}

float InputAsset::shrinkFactor() const noexcept {
    return _content->shrinkFactor();
}

float InputAsset::growFactor() const noexcept {
    return _content->growFactor();
}

math::vec2 InputAsset::outerSize() const noexcept {
    return _content->outerSize();
}

math::vec2 InputAsset::innerSize() const noexcept {
    return _content->innerSize();
}

math::vec2 InputAsset::screenOffset() const noexcept {
    return _content->screenOffset();
}

void InputAsset::enable() {
    Input<asset_guid>::enable();
    _input->enable();
}

void InputAsset::disable() {
    Input<asset_guid>::disable();
    _input->disable();
}

void InputAsset::markAsDirty() {
    Input<asset_guid>::markAsDirty();
    _input->markAsDirty();
}

void InputAsset::markAsPristine() {
    Input<asset_guid>::markAsPristine();
    _input->markAsPristine();
}

void InputAsset::markAsTouched() {
    Input<asset_guid>::markAsTouched();
    _input->markAsTouched();
}

void InputAsset::markAsUntouched() {
    Input<asset_guid>::markAsUntouched();
    _input->markAsUntouched();
}

void InputAsset::reset() {
    Input<asset_guid>::reset();
    _input->reset();
}

void InputAsset::updateValueAndValidity(const bool propagate_, const bool emit_) {
    Input<asset_guid>::updateValueAndValidity(propagate_, emit_);
    _input->updateValueAndValidity();
}

Input<asset_guid>::input_type InputAsset::value() const noexcept {
    return _value;
}

void InputAsset::setValue(cref<asset_guid> assetGuid_) {
    _value = assetGuid_;
    _input->setPlaceholder(_STD format(R"(<<{}-{}-{}-{}>>)", _value.pre, _value.c0, _value.c1, _value.post));

    /**/

    const auto query = Ember::assets()[assetGuid_];
    if (query.flags & AssetDatabaseResultType::eSuccess) {

        const auto name { query.value.getAssetName() };
        if (name.empty()) {
            return;
        }

        _input->setPlaceholder(string { name });
    }
}
