#include "InputAsset.hpp"

#include <algorithm>
#include <ranges>
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
#include <Engine.Reflow.Uikit/Atom/Layout/BoxLayout.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/HorizontalLayout.hpp>
#include <Engine.Resource/LoaderManager.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Heliogrim/AssetDatabase.hpp>
#include <Heliogrim/AssetDatabaseResult.hpp>
#include <Heliogrim/Heliogrim.hpp>

#include "../../Helper/AssetBrowserHelper.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

InputAsset::InputAsset() :
	Input(),
	_value(invalid_asset_guid) {}

InputAsset::~InputAsset() = default;

void InputAsset::setup() {

	_content = make_sptr<uikit::VerticalLayout>();
	//_content->attr.minWidth.setValue({ ReflowUnitType::eAbsolute, 72.F + 4.F + 16.F });
	//_content->attr.width.setValue({ ReflowUnitType::eRelative, 1.F });
	//_content->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 72.F + 4.F + 20.F });
	//_content->attr.maxHeight.setValue({ ReflowUnitType::eAbsolute, 72.F + 4.F + 20.F });
	//_content->attr.colGap.setValue(4.F);

	_content->setParent(shared_from_this());
	_children.setChild(_content);

	/**/

	auto upper { make_sptr<uikit::HorizontalLayout>() }; {
		auto& attr = upper->getLayoutAttributes().attributeSets;
		//attr.maxWidth.setValue({ ReflowUnitType::eAbsolute, 72.F + 4.F + 16.F });
		std::get<0>(attr).update<attr::BoxLayout::minWidth>({ ReflowUnitType::eAbsolute, 72.F + 4.F + 16.F });
		std::get<0>(attr).update<attr::BoxLayout::minHeight>({ ReflowUnitType::eAbsolute, 72.F });
		std::get<0>(attr).update<attr::BoxLayout::minHeight>({ ReflowUnitType::eAbsolute, 72.F });
		std::get<0>(attr).update<attr::BoxLayout::maxHeight>({ ReflowUnitType::eAbsolute, 72.F });
		std::get<1>(attr).update<attr::FlexLayout::rowGap>(4.F);
		std::get<1>(attr).update<attr::FlexLayout::justify>(ReflowSpacing::eSpaceAround);
		std::get<1>(attr).update<attr::FlexLayout::align>(ReflowAlignment::eCenter);
	}
	_content->addChild(upper);

	auto previewBox { make_sptr<uikit::BoxLayout>() };
	previewBox->getLayoutAttributes().update<attr::BoxLayout::minWidth>({ ReflowUnitType::eAbsolute, 72.F });
	previewBox->getLayoutAttributes().update<attr::BoxLayout::maxWidth>({ ReflowUnitType::eAbsolute, 72.F });
	previewBox->getLayoutAttributes().update<attr::BoxLayout::minHeight>({ ReflowUnitType::eAbsolute, 72.F });
	previewBox->getLayoutAttributes().update<attr::BoxLayout::maxHeight>({ ReflowUnitType::eAbsolute, 72.F });
	upper->addChild(previewBox);

	_preview = make_sptr<uikit::Image>();
	// TODO: theme->applyIcon72(_preview);

	auto* previewAsset = AssetBrowserHelper::get()->getItemIconByAssetType(asset_type_id {});

	auto iconRes = engine::Engine::getEngine()->getResources()->loader().loadImmediately<engine::assets::TextureAsset,
		engine::gfx::TextureResource>(std::move(previewAsset));

	auto previewGuard = iconRes->acquire(engine::resource::ResourceUsageFlag::eRead);

	/**/

	_preview->getDataAttributes().update<attr::ImageData::image>(*previewGuard.imm());
	_preview->getDataAttributes().update<attr::ImageData::resource>(iconRes.get());
	previewBox->setChild(_preview);

	auto actions { make_sptr<uikit::VerticalLayout>() };
	//actions->attr.minWidth.setValue({ ReflowUnitType::eAbsolute, 16.F });
	//actions->attr.maxWidth.setValue({ ReflowUnitType::eRelative, 16.F });
	//actions->attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 72.F });
	//actions->attr.maxHeight.setValue({ ReflowUnitType::eAbsolute, 72.F });
	//actions->attr.colGap.setValue(4.F);
	upper->addChild(actions);

	/**/

	auto txt { make_sptr<uikit::Text>() };
	txt->setText("S");
	_search = uikit::makeButton(uikit::TextButtonCreateOptions { .level = 2, .text = ::hg::move(txt) });
	actions->addChild(_search);

	txt = make_sptr<uikit::Text>();
	txt->setText("R");
	_reset = uikit::makeButton(uikit::TextButtonCreateOptions { .level = 2, .text = ::hg::move(txt) });
	actions->addChild(_reset);

	/**/

	auto lower { make_sptr<uikit::HorizontalLayout>() }; {
		auto& attr = lower->getLayoutAttributes().attributeSets;
		std::get<0>(attr).update<attr::BoxLayout::minWidth>({ ReflowUnitType::eAbsolute, 72.F + 4.F + 16.F });
		std::get<0>(attr).update<attr::BoxLayout::maxWidth>({ ReflowUnitType::eRelative, 1.F });
		std::get<0>(attr).update<attr::BoxLayout::minHeight>({ ReflowUnitType::eAbsolute, 20.F });
		std::get<0>(attr).update<attr::BoxLayout::maxHeight>({ ReflowUnitType::eAbsolute, 20.F });
	}

	_content->addChild(lower);

	_input = make_sptr<InputText>(); {
		auto& attr = _input->_wrapper->getLayoutAttributes().attributeSets;
		std::get<0>(attr).update<attr::BoxLayout::minWidth>({ ReflowUnitType::eRelative, 1.F });
		std::get<0>(attr).update<attr::BoxLayout::maxWidth>({ ReflowUnitType::eRelative, 1.F });
		std::get<0>(attr).update<attr::BoxLayout::minHeight>({ ReflowUnitType::eAbsolute, 20.F });
		std::get<0>(attr).update<attr::BoxLayout::maxHeight>({ ReflowUnitType::eAbsolute, 20.F });
		std::get<0>(attr).update<attr::BoxLayout::padding>(Padding { 4.F, 2.F });
	}

	_input->_text->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
	_input->_text->getStyleAttributes().update<attr::TextStyle::textEllipse>(1uL);

	lower->addChild(_input);
}

string InputAsset::getTag() const noexcept {
	return std::format(R"(InputAsset <{:#x}>)", reinterpret_cast<u64>(this));
}

EventResponse InputAsset::invokeOnDrop(ref<const DragDropEvent> event_) {

	if (not event_._payload.is<engine::input::event::DragDropEventTextPayload>()) {
		return Input<asset_guid>::invokeOnDrop(event_);
	}

	/**/

	const auto& payload = event_._payload.as<engine::input::event::DragDropEventTextPayload>();
	const auto& text = payload.data;

	if (not text.starts_with("asset://")) {
		return Input<asset_guid>::invokeOnDrop(event_);
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

	if (asset == None) {
		IM_CORE_WARNF("Unknown asset guid `{}`.", static_cast<String>(parsed.path()));
		return EventResponse::eConsumed;
	}

	if (not std::ranges::contains(_acceptedTypes, (*asset)->getTypeId())) {
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

EventResponse InputAsset::invokeOnDragOver(ref<const DragDropEvent> event_) {
	return Input<asset_guid>::invokeOnDragOver(event_);
}

const ptr<const Children> InputAsset::children() const {
	return &_children;
}

void InputAsset::render(const ptr<ReflowCommandBuffer> cmd_) {
	_content->render(cmd_);
}

PrefetchSizing InputAsset::prefetchSizing(ReflowAxis axis_, ref<const ReflowState> state_) const {
	::hg::todo_panic();
}

engine::reflow::PassPrefetchSizing InputAsset::passPrefetchSizing(
	engine::reflow::ReflowAxis axis_,
	ref<const engine::reflow::ReflowPassState> passState_
) const {
	::hg::todo_panic();
}

void InputAsset::computeSizing(ReflowAxis axis_, ref<const ReflowPassState> passState_) {
	::hg::todo_panic();
}

void InputAsset::applyLayout(ref<ReflowState> state_) {
	::hg::todo_panic();
}

math::fvec2 InputAsset::getShrinkFactor() const noexcept {
	return _content->getShrinkFactor();
}

math::fvec2 InputAsset::getGrowFactor() const noexcept {
	return _content->getGrowFactor();
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
