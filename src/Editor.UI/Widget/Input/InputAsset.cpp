#include "InputAsset.hpp"

#include <Engine.Common/Make.hpp>
#include "../../Color/Dark.hpp"
#include "../../Helper/AssetBrowserHelper.hpp"

#include "Engine.GFX.Glow.UI/TestUI.hpp"
#include "Engine.GFX.Loader/Texture/TextureResource.hpp"
#include "Engine.Resource/LoaderManager.hpp"
#include "Engine.Resource/ResourceManager.hpp"
#include <Heliogrim/AssetDatabase.hpp>
#include <Heliogrim/AssetDatabaseResult.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.GFX.Loader/Texture/Traits.hpp>

#include "Heliogrim/Heliogrim.hpp"
#include <Engine.Filesystem/__fwd.hpp>
#include <Engine.Filesystem/Url.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Common/GuidFormat.hpp>

#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/Assets.hpp>

#include "Editor.UI/Theme/Theme.hpp"
#include "Engine.Reflow/Widget/VerticalPanel.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

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

    const auto* const theme = Theme::get();

    _content = make_sptr<VerticalPanel>();
    _content->attr.minWidth.setValue({ ReflowUnitType::eAbsolute, 72.F + 4.F + 16.F });
    _content->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
    _content->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 72.F + 4.F + 20.F });
    _content->attr.maxHeight.setValue({ ReflowUnitType::eAbsolute, 72.F + 4.F + 20.F });
    _content->attr.colGap.setValue(4.F);

    _children.push_back(_content);

    /**/

    auto upper { make_sptr<HorizontalPanel>() };
    upper->style() = BoundStyleSheet::make(
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
    );
    _content->addChild(upper);

    auto previewBox { make_sptr<BoxPanel>() };
    previewBox->attr.width.setValue({ ReflowUnitType::eAbsolute, 72.F });
    previewBox->attr.maxWidth.setValue({ ReflowUnitType::eAbsolute, 72.F });
    previewBox->attr.height.setValue({ ReflowUnitType::eAbsolute, 72.F });
    previewBox->attr.maxHeight.setValue({ ReflowUnitType::eAbsolute, 72.F });
    upper->addChild(previewBox);

    _preview = make_sptr<Image>();
    theme->applyIcon72(_preview);

    auto* previewAsset = AssetBrowserHelper::get()->getItemIconByAssetType(asset_type_id {});

    auto iconRes = engine::Engine::getEngine()->getResources()->loader().loadImmediately<engine::assets::Texture,
        engine::gfx::TextureResource>(_STD move(previewAsset));

    auto previewGuard = iconRes->acquire(engine::resource::ResourceUsageFlag::eRead);
    auto* view = previewGuard->as<engine::gfx::VirtualTextureView>();

    /**/

    _preview->setImage(make_sptr<engine::gfx::ProxyTexture<non_owning_rptr>>(_STD move(view)), iconRes.get());
    previewBox->setChild(_preview);

    auto actions { make_sptr<VerticalPanel>() };
    actions->attr.minWidth.setValue({ ReflowUnitType::eAbsolute, 16.F });
    actions->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 16.F });
    actions->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 72.F });
    actions->attr.maxHeight.setValue({ ReflowUnitType::eAbsolute, 72.F });
    actions->attr.colGap.setValue(4.F);
    upper->addChild(actions);

    /**/

    _search = make_sptr<Button>();
    theme->applyTextButton(_search);
    actions->addChild(_search);

    auto txt { make_sptr<Text>() };
    theme->applyLabel(txt);
    txt->setText("S");
    _search->setChild(txt);

    _reset = make_sptr<Button>();
    theme->applyTextButton(_reset);
    actions->addChild(_reset);

    txt = make_sptr<Text>();
    theme->applyLabel(txt);
    txt->setText("R");
    _reset->setChild(txt);

    /**/

    auto lower { make_sptr<HorizontalPanel>() };
    lower->attr.minWidth.setValue({ ReflowUnitType::eAbsolute, 72.F + 4.F + 16.F });
    lower->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
    lower->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
    lower->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 20.F });
    lower->attr.maxHeight.setValue({ ReflowUnitType::eAbsolute, 20.F });

    _content->addChild(lower);

    _input = make_sptr<InputText>();
    _input->_wrapper->style() = makeInputBoxStyle();
    _input->_text->style() = makeInputTextStyle();
    lower->addChild(_input);
}

string InputAsset::getTag() const noexcept {
    return _STD format(R"(InputAsset <{:#x}>)", reinterpret_cast<u64>(this));
}

EventResponse InputAsset::onDrop(cref<DragDropEvent> event_) {

    if (event_._type != engine::input::DragDropObjectType::eTextType) {
        return Input<asset_guid>::onDrop(event_);
    }

    assert(event_._data._ != nullptr);
    const auto& text = event_._data.text->data;

    if (not text.starts_with("asset://")) {
        return Input<asset_guid>::onDrop(event_);
    }

    static constexpr auto pathIdx = sizeof("asset://") - 1;

    fs::Url parsed {
        "asset"sv,
        fs::Path { text.substr(pathIdx) }
    };

    if (parsed.path().empty() || parsed.path().hasParent()) {
        IM_CORE_WARN("Invalid encoded asset url.");
        return EventResponse::eConsumed;
    }

    const asset_guid guid = decodeGuid4228(parsed.path().string()).data;
    if (guid == invalid_asset_guid) {
        IM_CORE_WARN("Invalid asset guid.");
        return EventResponse::eConsumed;
    }

    /**/

    const auto* const asset = engine::Engine::getEngine()->getAssets()->getRegistry()->findAssetByGuid(guid);

    if (asset == nullptr) {
        IM_CORE_WARNF("Unknown asset guid `{}`.", parsed.path().string());
        return EventResponse::eConsumed;
    }

    if (not _STD ranges::contains(_acceptedTypes, asset->getTypeId())) {
        IM_CORE_WARN("Invalid asset type.");
        return EventResponse::eConsumed;
    }

    /**/

    setValue(guid);
    markAsDirty();

    /* Warning: Temporary */
    if (_callback) {
        _callback(guid);
    }
    /* !Warning: Temporary */

    return EventResponse::eConsumed;
}

EventResponse InputAsset::onDragOver(cref<DragDropEvent> event_) {
    return Input<asset_guid>::onDragOver(event_);
}

const ptr<const Children> InputAsset::children() const {
    return &_children;
}

void InputAsset::render(cref<ReflowState> state_, const ptr<ReflowCommandBuffer> cmd_) {
    _content->render(state_, cmd_);
}

math::vec2 InputAsset::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {
    return _content->getDesiredSize();
}

math::vec2 InputAsset::computeDesiredSize(cref<engine::reflow::ReflowPassState> passState_) const {
    return _content->getDesiredSize();
}

void InputAsset::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {
    auto* const childState = state_.getStateOf(_content);
    childState->layoutOffset = ctx_.localOffset;
    childState->layoutSize = ctx_.localSize;
}

float InputAsset::shrinkFactor() const noexcept {
    return _content->shrinkFactor();
}

float InputAsset::growFactor() const noexcept {
    return _content->growFactor();
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

    const auto query = Heliogrim::assets()[assetGuid_];
    if (query.flags & AssetDatabaseResultType::eSuccess) {

        const auto name { query.value.getAssetName() };
        if (name.empty()) {
            return;
        }

        _input->setPlaceholder(string { name });
    }
}

cref<Vector<asset_type_id>> InputAsset::acceptedTypes() const noexcept {
    return _acceptedTypes;
}

void InputAsset::addAcceptedType(asset_type_id typeId_) {
    if (_STD ranges::contains(_acceptedTypes, typeId_)) {
        return;
    }

    _acceptedTypes.push_back(typeId_);
}

void InputAsset::dropAcceptedType(asset_type_id typeId_) {
    const auto sub = _STD ranges::remove(_acceptedTypes, typeId_);
    _acceptedTypes.erase(sub.end(), _acceptedTypes.end());
}
