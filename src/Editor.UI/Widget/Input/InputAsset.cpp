#include "InputAsset.hpp"

#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Common/GuidFormat.hpp>
#include <Engine.Common/Make.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Filesystem/Url.hpp>
#include <Engine.Filesystem/__fwd.hpp>
#include <Engine.GFX.Loader/Texture/TextureResource.hpp>
#include <Engine.GFX.Loader/Texture/Traits.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Reflow/Widget/VerticalPanel.hpp>
#include <Engine.Resource/LoaderManager.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Heliogrim/AssetDatabase.hpp>
#include <Heliogrim/AssetDatabaseResult.hpp>
#include <Heliogrim/Heliogrim.hpp>

#include "../../Color/Dark.hpp"
#include "../../Helper/AssetBrowserHelper.hpp"
#include "../../Theme/Theme.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

InputAsset::InputAsset() :
	Input(),
	_value(invalid_asset_guid) {}

InputAsset::~InputAsset() = default;

void InputAsset::setup() {

	const auto theme = Theme::get();

	_content = make_sptr<VerticalPanel>();
	_content->attr.minWidth.setValue({ ReflowUnitType::eAbsolute, 72.F + 4.F + 16.F });
	_content->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
	_content->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 72.F + 4.F + 20.F });
	_content->attr.maxHeight.setValue({ ReflowUnitType::eAbsolute, 72.F + 4.F + 20.F });
	_content->attr.colGap.setValue(4.F);

	_content->setParent(shared_from_this());
	_children.setChild(_content);

	/**/

	auto upper { make_sptr<HorizontalPanel>() }; {
		auto& attr = upper->attr;
		attr.minWidth.setValue({ ReflowUnitType::eAbsolute, 72.F + 4.F + 16.F });
		attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
		//attr.maxWidth.setValue({ ReflowUnitType::eAbsolute, 72.F + 4.F + 16.F });
		attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 72.F });
		attr.height.setValue({ ReflowUnitType::eAbsolute, 72.F });
		attr.maxHeight.setValue({ ReflowUnitType::eAbsolute, 72.F });
		attr.rowGap.setValue(4.F);
		attr.justify.setValue(ReflowSpacing::eSpaceAround);
		attr.align.setValue(ReflowAlignment::eCenter);
	}
	_content->addChild(upper);

	auto previewBox { make_sptr<BoxPanel>() };
	previewBox->attributes().width.setValue({ ReflowUnitType::eAbsolute, 72.F });
	previewBox->attributes().maxWidth.setValue({ ReflowUnitType::eAbsolute, 72.F });
	previewBox->attributes().height.setValue({ ReflowUnitType::eAbsolute, 72.F });
	previewBox->attributes().maxHeight.setValue({ ReflowUnitType::eAbsolute, 72.F });
	upper->addChild(previewBox);

	_preview = make_sptr<Image>();
	theme->applyIcon72(_preview);

	auto* previewAsset = AssetBrowserHelper::get()->getItemIconByAssetType(asset_type_id {});

	auto iconRes = engine::Engine::getEngine()->getResources()->loader().loadImmediately<engine::assets::TextureAsset,
		engine::gfx::TextureResource>(std::move(previewAsset));

	auto previewGuard = iconRes->acquire(engine::resource::ResourceUsageFlag::eRead);

	/**/

	_preview->setImage(*previewGuard.imm(), iconRes.get());
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

	_input = make_sptr<InputText>(); {
		auto& attr = _input->_wrapper->attr;
		attr.minWidth.setValue({ ReflowUnitType::eRelative, 1.F });
		attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
		attr.maxWidth.setValue({ ReflowUnitType::eRelative, 1.F });
		attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 20.F });
		attr.height.setValue({ ReflowUnitType::eAbsolute, 20.F });
		attr.maxHeight.setValue({ ReflowUnitType::eAbsolute, 20.F });
		attr.padding.setValue(Padding { 4.F, 2.F });
		attr.flexShrink.setValue(1.F);
	}

	theme->applyText(_input->_text);
	_input->_text->attr.textAlign.setValue(TextAlign::eLeftMiddle);
	_input->_text->attr.textEllipse.setValue(1uL);

	lower->addChild(_input);
}

string InputAsset::getTag() const noexcept {
	return std::format(R"(InputAsset <{:#x}>)", reinterpret_cast<u64>(this));
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

	const asset_guid guid = decodeGuid4228(static_cast<String>(parsed.path()));
	if (guid == invalid_asset_guid) {
		IM_CORE_WARN("Invalid asset guid.");
		return EventResponse::eConsumed;
	}

	/**/

	const auto asset = engine::Engine::getEngine()->getAssets()->getRegistry()->findAssetByGuid(guid);

	if (asset == nullptr) {
		IM_CORE_WARNF("Unknown asset guid `{}`.", static_cast<String>(parsed.path()));
		return EventResponse::eConsumed;
	}

	if (not std::ranges::contains(_acceptedTypes, asset->getTypeId())) {
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

void InputAsset::render(const ptr<ReflowCommandBuffer> cmd_) {
	_content->render(cmd_);
}

math::vec2 InputAsset::prefetchDesiredSize(cref<ReflowState> state_, float scale_) const {
	return _content->prefetchDesiredSize(state_, scale_);
}

math::vec2 InputAsset::computeDesiredSize(cref<engine::reflow::ReflowPassState> passState_) const {
	return _content->computeDesiredSize(passState_);
}

void InputAsset::applyLayout(ref<ReflowState> state_, mref<LayoutContext> ctx_) {
	const auto childState = state_.getStateOf(_content);
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
	_input->setPlaceholder(std::format(R"(<<{}-{}-{}-{}>>)", _value.pre, _value.c0, _value.c1, _value.post));

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
	if (std::ranges::contains(_acceptedTypes, typeId_)) {
		return;
	}

	_acceptedTypes.push_back(typeId_);
}

void InputAsset::dropAcceptedType(asset_type_id typeId_) {
	const auto sub = std::ranges::remove(_acceptedTypes, typeId_);
	_acceptedTypes.erase(sub.end(), _acceptedTypes.end());
}
