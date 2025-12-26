#include <Engine.Assets.Type/Geometry/StaticGeometry.hpp>
#include <Engine.Assets.Type/Material/GfxMaterial.hpp>
#include <Engine.Assets.Type/Material/GfxMaterialPrototype.hpp>

/**/

#include "PropertyServiceInit.hpp"

#include <Engine.Reflect/IsType.hpp>
#include <Engine.Reflect/TypeId.hpp>
#include <Engine.Reflow.Uikit/Atom/Text.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/HorizontalLayout.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/UniformGridLayout.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/VerticalLayout.hpp>
#include <Heliogrim/Component/Scene/Camera/CameraComponent.hpp>
#include <Heliogrim/Component/Scene/Geometry/StaticGeometryComponent.hpp>
#include <Heliogrim/Component/Scene/Light/DirectionalLightComponent.hpp>
#include <Heliogrim/Component/Scene/Light/PointLightComponent.hpp>
#include <Heliogrim/Component/Scene/Light/SpotLightComponent.hpp>
#include <Heliogrim/Component/Scene/Sky/SkyboxComponent.hpp>

/**/

#include <Editor.UI/Event/SelectMenuEvent.hpp>
#include <Editor.UI/Widget/Input/InputFloat.hpp>
#include <Editor.UI/Widget/Input/InputIntegral.hpp>
#include <Editor.UI/Widget/Input/InputSelect.hpp>
#include <Editor.UI/Widget/Input/InputText.hpp>
#include <Editor.UI/Widget/Input/InputToggle.hpp>
#include <Engine.Assets.System/AssetRegistry.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Common/Discard.hpp>
#include <Engine.Common/GuidFormat.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Reflow.Uikit/Atom/Layout/BoxLayout.hpp>
#include <Heliogrim/AssetDatabase.hpp>
#include <Heliogrim/Heliogrim.hpp>
#include "PropertyService.hpp"

using namespace hg::editor::ui::service;
using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg::engine;
using namespace hg;

void service::initPropertyService(ref<PropertyService> service_) {

	service_.registerProperty(
		{
			.debugName = "Actor :: Transform",
			.section = "Global Transform",
			.selector = [](Property::selector_fn_arg data_) {
				return data_.is<nmpt<const Actor>>() && data_.as<nmpt<const Actor>>()->getRootComponent().has_value();
			},
			.view = [](Property::view_fn_arg data_) {
				auto root = data_.as<nmpt<Actor>>()->getRootComponent();
				auto grid = make_sptr<uikit::UniformGridLayout>();

				std::get<0>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
				std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minHeight>(
					{ ReflowUnitType::eAbsolute, 20.F }
				);
				std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxHeight>(
					{ ReflowUnitType::eAbsolute, 20.F }
				);
				std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minWidth>(
					{ ReflowUnitType::eRelative, 1.F / 4.F }
				);
				std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxWidth>(
					{ ReflowUnitType::eRelative, 1.F / 4.F }
				);

				/**/

				auto posLabel = make_sptr<uikit::Text>();
				posLabel->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
				posLabel->setText("Position");

				auto inputPosX = makeInputFloat();
				inputPosX->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
				inputPosX->getDataAttributes().setQuery<attr::InputData::value>(
					[root]() {
						return root->getUniverseLocation().into().x;
					}
				);
				discard = inputPosX->onInputChange(
					[root](ref<const InputChangeEvent> event_) mutable {
						const auto& target = *static_cast<const ptr<const InputFloat>>(event_.target.get());
						root->getLocalTransform().location().setX(target.value());
						return EventResponse::eConsumed;
					}
				);

				auto inputPosY = makeInputFloat();
				inputPosY->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
				inputPosY->getDataAttributes().setQuery<attr::InputData::value>(
					[root]() {
						return root->getUniverseLocation().into().y;
					}
				);
				discard = inputPosY->onInputChange(
					[root](ref<const InputChangeEvent> event_) mutable {
						const auto& target = *static_cast<const ptr<const InputFloat>>(event_.target.get());
						root->getLocalTransform().location().setY(target.value());
						return EventResponse::eConsumed;
					}
				);

				auto inputPosZ = makeInputFloat();
				inputPosZ->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
				inputPosZ->getDataAttributes().setQuery<attr::InputData::value>(
					[root]() {
						return root->getUniverseLocation().into().z;
					}
				);
				discard = inputPosZ->onInputChange(
					[root](ref<const InputChangeEvent> event_) mutable {
						const auto& target = *static_cast<const ptr<const InputFloat>>(event_.target.get());
						root->getLocalTransform().location().setZ(target.value());
						return EventResponse::eConsumed;
					}
				);

				grid->addChild(hg::move(posLabel));
				for (auto&& item : { inputPosX, inputPosY, inputPosZ }) {
					grid->addChild(hg::move(item));
				}

				/**/

				auto rotLabel = make_sptr<uikit::Text>();
				rotLabel->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
				rotLabel->setText("Rotation");

				auto inputRotX = makeInputFloat();
				inputRotX->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
				inputRotX->setValue(root->getUniverseRotator().euler().x);

				auto inputRotY = makeInputFloat();
				inputRotY->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
				inputRotY->setValue(root->getUniverseRotator().euler().y);

				auto inputRotZ = makeInputFloat();
				inputRotZ->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
				inputRotZ->setValue(root->getUniverseRotator().euler().z);

				grid->addChild(hg::move(rotLabel));
				for (auto&& item : { inputRotX, inputRotY, inputRotZ }) {
					grid->addChild(hg::move(item));
				}

				/**/

				return grid;
			}
		}
	);

	service_.registerProperty(
		{
			.debugName = "Component :: Transform",
			.section = "Local Transform",
			.selector = [](Property::selector_fn_arg data_) {
				return data_.is<nmpt<const HierarchyComponent>>() && data_.as<nmpt<const HierarchyComponent>>() != nullptr;
			},
			.view = [](Property::view_fn_arg data_) {
				const auto component = data_.as<nmpt<HierarchyComponent>>();
				auto grid = make_sptr<uikit::UniformGridLayout>();

				std::get<0>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
				std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minHeight>(
					{ ReflowUnitType::eAbsolute, 20.F }
				);
				std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxHeight>(
					{ ReflowUnitType::eAbsolute, 20.F }
				);
				std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minWidth>(
					{ ReflowUnitType::eRelative, 1.F / 4.F }
				);
				std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxWidth>(
					{ ReflowUnitType::eRelative, 1.F / 4.F }
				);

				auto posLabel = make_sptr<uikit::Text>();
				posLabel->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
				posLabel->setText("Position");

				auto inputPosX = makeInputFloat();
				inputPosX->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
				inputPosX->getDataAttributes().setQuery<attr::InputData::value>(
					[component]() {
						return component->getLocalTransform().location().into().x;
					}
				);
				discard = inputPosX->onInputChange(
					[component](ref<const InputChangeEvent> event_) {
						const auto& target = *static_cast<const ptr<const InputFloat>>(event_.target.get());
						component->getLocalTransform().location().setX(target.value());
						return EventResponse::eConsumed;
					}
				);

				auto inputPosY = makeInputFloat();
				inputPosY->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
				inputPosY->getDataAttributes().setQuery<attr::InputData::value>(
					[component]() {
						return component->getLocalTransform().location().into().y;
					}
				);
				discard = inputPosY->onInputChange(
					[component](ref<const InputChangeEvent> event_) {
						const auto& target = *static_cast<const ptr<const InputFloat>>(event_.target.get());
						component->getLocalTransform().location().setY(target.value());
						return EventResponse::eConsumed;
					}
				);

				auto inputPosZ = makeInputFloat();
				inputPosZ->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
				inputPosZ->getDataAttributes().setQuery<attr::InputData::value>(
					[component]() {
						return component->getLocalTransform().location().into().z;
					}
				);
				discard = inputPosZ->onInputChange(
					[component](ref<const InputChangeEvent> event_) {
						const auto& target = *static_cast<const ptr<const InputFloat>>(event_.target.get());
						component->getLocalTransform().location().setZ(target.value());
						return EventResponse::eConsumed;
					}
				);

				grid->addChild(hg::move(posLabel));
				for (auto&& item : { inputPosX, inputPosY, inputPosZ }) {
					grid->addChild(hg::move(item));
				}

				/**/

				auto rotLabel = make_sptr<uikit::Text>();
				rotLabel->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
				rotLabel->setText("Rotation");

				auto inputRotX = makeInputFloat();
				inputRotX->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
				inputRotX->getDataAttributes().setQuery<attr::InputData::value>(
					[component]() {
						return component->getLocalTransform().rotator().euler().x;
					}
				);
				discard = inputRotX->onInputChange(
					[component](ref<const InputChangeEvent> event_) {
						const auto& target = *static_cast<const ptr<const InputFloat>>(event_.target.get());
						component->getLocalTransform().rotator().setPitch(target.value());
						return EventResponse::eConsumed;
					}
				);

				auto inputRotY = makeInputFloat();
				inputRotY->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
				inputRotY->getDataAttributes().setQuery<attr::InputData::value>(
					[component]() {
						return component->getLocalTransform().rotator().euler().y;
					}
				);
				discard = inputRotY->onInputChange(
					[component](ref<const InputChangeEvent> event_) {
						const auto target = std::static_pointer_cast<InputFloat>(event_.target);
						component->getLocalTransform().rotator().setYaw(target->value());
						return EventResponse::eConsumed;
					}
				);

				auto inputRotZ = makeInputFloat();
				inputRotZ->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
				inputRotZ->getDataAttributes().setQuery<attr::InputData::value>(
					[component]() {
						return component->getLocalTransform().rotator().euler().z;
					}
				);
				discard = inputRotZ->onInputChange(
					[component](ref<const InputChangeEvent> event_) {
						const auto& target = *static_cast<const ptr<const InputFloat>>(event_.target.get());
						component->getLocalTransform().rotator().setRoll(target.value());
						return EventResponse::eConsumed;
					}
				);

				grid->addChild(hg::move(rotLabel));
				for (auto&& item : { inputRotX, inputRotY, inputRotZ }) {
					grid->addChild(hg::move(item));
				}

				/**/

				auto scaleLabel = make_sptr<uikit::Text>();
				scaleLabel->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
				scaleLabel->setText("Scale");

				auto inputScaleX = makeInputFloat();
				inputScaleX->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
				inputScaleX->getDataAttributes().setQuery<attr::InputData::value>(
					[component]() {
						return component->getLocalTransform().scale().x;
					}
				);
				discard = inputScaleX->onInputChange(
					[component](ref<const InputChangeEvent> event_) {
						const auto& target = *static_cast<const ptr<const InputFloat>>(event_.target.get());
						component->getLocalTransform().scale().setX(target.value());
						return EventResponse::eConsumed;
					}
				);

				auto inputScaleY = makeInputFloat();
				inputScaleY->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
				inputScaleY->getDataAttributes().setQuery<attr::InputData::value>(
					[component]() {
						return component->getLocalTransform().scale().y;
					}
				);
				discard = inputScaleY->onInputChange(
					[component](ref<const InputChangeEvent> event_) {
						const auto& target = *static_cast<const ptr<const InputFloat>>(event_.target.get());
						component->getLocalTransform().scale().setY(target.value());
						return EventResponse::eConsumed;
					}
				);

				auto inputScaleZ = makeInputFloat();
				inputScaleZ->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
				inputScaleZ->getDataAttributes().setQuery<attr::InputData::value>(
					[component]() {
						return component->getLocalTransform().scale().z;
					}
				);
				discard = inputScaleZ->onInputChange(
					[component](ref<const InputChangeEvent> event_) {
						const auto& target = *static_cast<const ptr<const InputFloat>>(event_.target.get());
						component->getLocalTransform().scale().setZ(target.value());
						return EventResponse::eConsumed;
					}
				);

				grid->addChild(hg::move(scaleLabel));
				for (auto&& item : { inputScaleX, inputScaleY, inputScaleZ }) {
					grid->addChild(hg::move(item));
				}

				/**/

				return grid;
			}
		}
	);

	service_.registerProperty(
		{
			.debugName = "Camera Component",
			.section = "Camera",
			.selector = [](Property::selector_fn_arg data_) {
				return data_.is<nmpt<const HierarchyComponent>>() && data_.as<nmpt<const HierarchyComponent>>() != nullptr &&
					IsType<CameraComponent>(*data_.as<nmpt<const HierarchyComponent>>());
			},
			.view = [](Property::view_fn_arg data_) {
				const auto component = Cast<CameraComponent>(data_.as<nmpt<HierarchyComponent>>());

				auto list = make_sptr<uikit::VerticalLayout>();
				std::get<0>(list->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);

				/**/

				{
					auto row = make_sptr<uikit::HorizontalLayout>();
					std::get<0>(row->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
					std::get<1>(row->getLayoutAttributes().attributeSets).update<attr::FlexLayout::justify>(
						ReflowSpacing::eSpaceBetween
					);

					auto label = make_sptr<uikit::Text>();
					label->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
					label->setText("Aspect");

					auto input = makeInputFloat();
					input->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
					input->setValue(component->getAspectRatio());

					row->addChild(hg::move(label));
					row->addChild(hg::move(input));

					list->addChild(hg::move(row));
				}

				/**/

				{
					auto row = make_sptr<uikit::HorizontalLayout>();
					std::get<0>(row->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
					std::get<1>(row->getLayoutAttributes().attributeSets).update<attr::FlexLayout::justify>(
						ReflowSpacing::eSpaceBetween
					);

					auto label = make_sptr<uikit::Text>();
					label->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
					label->setText("Ratio Lock");

					auto active = make_sptr<uikit::Text>();
					active->setText("Locked");

					auto inactive = make_sptr<uikit::Text>();
					inactive->setText("Free");

					auto input = makeInputToggle(::hg::move(active), ::hg::move(inactive), false);
					input->getDataAttributes().setQuery<attr::InputData::value>(
						[component]() {
							return component->isAspectLocked();
						}
					);
					discard = input->onInputChange(
						[component](ref<const InputChangeEvent> event_) {
							const auto& target = *static_cast<const ptr<const InputToggle>>(event_.target.get());
							component->lockAspectRatio(target.value());
							return EventResponse::eConsumed;
						}
					);

					auto box = make_sptr<uikit::BoxLayout>();
					box->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
					box->setChild(::hg::move(input));

					row->addChild(hg::move(label));
					row->addChild(hg::move(box));

					list->addChild(hg::move(row));
				}

				/**/

				{
					auto grid = make_sptr<uikit::UniformGridLayout>();

					std::get<0>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minHeight>(
						{ ReflowUnitType::eAbsolute, 20.F }
					);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxHeight>(
						{ ReflowUnitType::eAbsolute, 20.F }
					);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minWidth>(
						{ ReflowUnitType::eRelative, 1.F / 4.F }
					);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxWidth>(
						{ ReflowUnitType::eRelative, 1.F / 4.F }
					);

					auto label = make_sptr<uikit::Text>();
					label->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
					label->setText("Offset");

					auto inputX = makeInputFloat();
					inputX->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
					inputX->setValue(component->offset().x);

					auto inputY = makeInputFloat();
					inputY->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
					inputY->setValue(component->offset().y);

					auto inputZ = makeInputFloat();
					inputZ->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
					inputZ->setValue(component->offset().z);

					grid->addChild(hg::move(label));
					for (auto&& item : { inputX, inputY, inputZ }) {
						grid->addChild(hg::move(item));
					}

					list->addChild(hg::move(grid));
				}

				/**/

				{
					auto row = make_sptr<uikit::HorizontalLayout>();
					std::get<0>(row->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
					std::get<1>(row->getLayoutAttributes().attributeSets).update<attr::FlexLayout::justify>(
						ReflowSpacing::eSpaceBetween
					);

					auto label = make_sptr<uikit::Text>();
					label->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
					label->setText("Projection");

					auto active = make_sptr<uikit::Text>();
					active->setText("Orthographic");

					auto inactive = make_sptr<uikit::Text>();
					inactive->setText("Perspective");

					auto input = makeInputToggle(::hg::move(active), ::hg::move(inactive), false);
					input->getDataAttributes().setQuery<attr::InputData::value>(
						[component]() {
							return component->getProjectMode() == gfx::CameraProjectionMode::eOrthographic;
						}
					);
					discard = input->onInputChange(
						[component](ref<const InputChangeEvent> event_) {
							const auto& target = *static_cast<const ptr<const InputToggle>>(event_.target.get());
							component->setProjectMode(
								target.value() ? gfx::CameraProjectionMode::eOrthographic : gfx::CameraProjectionMode::ePerspective
							);
							return EventResponse::eConsumed;
						}
					);

					auto box = make_sptr<uikit::BoxLayout>();
					box->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
					box->setChild(::hg::move(input));

					row->addChild(hg::move(label));
					row->addChild(hg::move(box));

					list->addChild(hg::move(row));
				}

				/**/

				{
					auto row = make_sptr<uikit::HorizontalLayout>();
					std::get<0>(row->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
					std::get<1>(row->getLayoutAttributes().attributeSets).update<attr::FlexLayout::justify>(
						ReflowSpacing::eSpaceBetween
					);

					auto label = make_sptr<uikit::Text>();
					label->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
					label->setText("View Mode");

					auto active = make_sptr<uikit::Text>();
					active->setText("Look At");

					auto inactive = make_sptr<uikit::Text>();
					inactive->setText("First Person");

					auto input = makeInputToggle(::hg::move(active), ::hg::move(inactive), false);
					input->getDataAttributes().setQuery<attr::InputData::value>(
						[component]() {
							return component->getViewMode() == gfx::CameraViewMode::eLookAt;
						}
					);
					discard = input->onInputChange(
						[component](ref<const InputChangeEvent> event_) {
							const auto& target = *static_cast<const ptr<const InputToggle>>(event_.target.get());
							component->setViewMode(target.value() ? gfx::CameraViewMode::eLookAt : gfx::CameraViewMode::eFirstPerson);
							return EventResponse::eConsumed;
						}
					);

					auto box = make_sptr<uikit::BoxLayout>();
					box->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
					box->setChild(::hg::move(input));

					row->addChild(hg::move(label));
					row->addChild(hg::move(box));

					list->addChild(hg::move(row));
				}

				/**/

				return list;
			}
		}
	);

	service_.registerProperty(
		{
			.debugName = "Directional Light Component :: Lighting",
			.section = "Lighting",
			.selector = [](Property::selector_fn_arg data_) {
				return data_.is<nmpt<const HierarchyComponent>>() && data_.as<nmpt<const HierarchyComponent>>() != nullptr &&
					IsType<DirectionalLightComponent>(*data_.as<nmpt<const HierarchyComponent>>());
			},
			.view = [](Property::view_fn_arg data_) {
				const auto component = Cast<DirectionalLightComponent>(data_.as<nmpt<HierarchyComponent>>());

				auto list = make_sptr<uikit::VerticalLayout>();
				std::get<0>(list->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);

				/**/

				{
					auto grid = make_sptr<uikit::UniformGridLayout>();

					std::get<0>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minHeight>(
						{ ReflowUnitType::eAbsolute, 20.F }
					);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxHeight>(
						{ ReflowUnitType::eAbsolute, 20.F }
					);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minWidth>(
						{ ReflowUnitType::eRelative, 1.F / 4.F }
					);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxWidth>(
						{ ReflowUnitType::eRelative, 1.F / 4.F }
					);

					auto label = make_sptr<uikit::Text>();
					label->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
					label->setText("Luminance");

					auto inputR = makeInputFloat();
					inputR->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
					inputR->getDataAttributes().update<attr::InputData::limits>(math::fvec2 { 0.F, 1.F });
					inputR->getDataAttributes().setQuery<attr::InputData::value>(
						[component]() {
							return component->_luminance.r;
						}
					);
					::hg::discard = inputR->onInputChange(
						[component](ref<const InputChangeEvent> event_) {
							const auto& target = *static_cast<const ptr<const InputFloat>>(event_.target.get());
							component->_luminance.setX(target.value());
							return EventResponse::eConsumed;
						}
					);

					auto inputG = makeInputFloat();
					inputG->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
					inputG->getDataAttributes().update<attr::InputData::limits>(math::fvec2 { 0.F, 1.F });
					inputG->getDataAttributes().setQuery<attr::InputData::value>(
						[component]() {
							return component->_luminance.g;
						}
					);
					::hg::discard = inputG->onInputChange(
						[component](ref<const InputChangeEvent> event_) {
							const auto& target = *static_cast<const ptr<const InputFloat>>(event_.target.get());
							component->_luminance.setY(target.value());
							return EventResponse::eConsumed;
						}
					);

					auto inputB = makeInputFloat();
					inputB->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
					inputB->getDataAttributes().update<attr::InputData::limits>(math::fvec2 { 0.F, 1.F });
					inputB->getDataAttributes().setQuery<attr::InputData::value>(
						[component]() {
							return component->_luminance.b;
						}
					);
					::hg::discard = inputB->onInputChange(
						[component](ref<const InputChangeEvent> event_) {
							const auto& target = *static_cast<const ptr<const InputFloat>>(event_.target.get());
							component->_luminance.setZ(target.value());
							return EventResponse::eConsumed;
						}
					);

					grid->addChild(hg::move(label));
					for (auto&& item : { inputR, inputG, inputB }) {
						grid->addChild(hg::move(item));
					}

					list->addChild(hg::move(grid));
				}

				/**/

				{
					auto grid = make_sptr<uikit::UniformGridLayout>();

					std::get<0>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minHeight>(
						{ ReflowUnitType::eAbsolute, 20.F }
					);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxHeight>(
						{ ReflowUnitType::eAbsolute, 20.F }
					);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minWidth>(
						{ ReflowUnitType::eRelative, 1.F / 4.F }
					);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxWidth>(
						{ ReflowUnitType::eRelative, 1.F / 4.F }
					);

					auto label = make_sptr<uikit::Text>();
					label->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
					label->setText("Direction");

					auto inputX = makeInputFloat();
					inputX->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
					inputX->getDataAttributes().setQuery<attr::InputData::value>(
						[component]() {
							return component->_direction.pitch();
						}
					);
					::hg::discard = inputX->onInputChange(
						[component](ref<const InputChangeEvent> event_) {
							const auto& target = *static_cast<const ptr<const InputFloat>>(event_.target.get());
							component->_direction.setPitch(target.value());
							return EventResponse::eConsumed;
						}
					);

					auto inputY = makeInputFloat();
					inputY->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
					inputY->getDataAttributes().setQuery<attr::InputData::value>(
						[component]() {
							return component->_direction.yaw();
						}
					);
					::hg::discard = inputY->onInputChange(
						[component](ref<const InputChangeEvent> event_) {
							const auto& target = *static_cast<const ptr<const InputFloat>>(event_.target.get());
							component->_direction.setYaw(target.value());
							return EventResponse::eConsumed;
						}
					);

					auto inputZ = makeInputFloat();
					inputZ->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
					inputZ->getDataAttributes().setQuery<attr::InputData::value>(
						[component]() {
							return component->_direction.roll();
						}
					);
					::hg::discard = inputZ->onInputChange(
						[component](ref<const InputChangeEvent> event_) {
							const auto& target = *static_cast<const ptr<const InputFloat>>(event_.target.get());
							component->_direction.setRoll(target.value());
							return EventResponse::eConsumed;
						}
					);

					grid->addChild(hg::move(label));
					for (auto&& item : { inputX, inputY, inputZ }) {
						grid->addChild(hg::move(item));
					}

					list->addChild(hg::move(grid));
				}

				/**/

				return list;
			}
		}
	);

	service_.registerProperty(
		{
			.debugName = "Point Light Component :: Lighting",
			.section = "Lighting",
			.selector = [](Property::selector_fn_arg data_) {
				return data_.is<nmpt<const HierarchyComponent>>() && data_.as<nmpt<const HierarchyComponent>>() != nullptr &&
					IsType<PointLightComponent>(*data_.as<nmpt<const HierarchyComponent>>());
			},
			.view = [](Property::view_fn_arg data_) {
				const auto component = Cast<PointLightComponent>(data_.as<nmpt<HierarchyComponent>>());

				auto list = make_sptr<uikit::VerticalLayout>();
				std::get<0>(list->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);

				/**/

				{
					auto grid = make_sptr<uikit::UniformGridLayout>();

					std::get<0>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minHeight>(
						{ ReflowUnitType::eAbsolute, 20.F }
					);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxHeight>(
						{ ReflowUnitType::eAbsolute, 20.F }
					);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minWidth>(
						{ ReflowUnitType::eRelative, 1.F / 4.F }
					);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxWidth>(
						{ ReflowUnitType::eRelative, 1.F / 4.F }
					);

					auto label = make_sptr<uikit::Text>();
					label->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
					label->setText("Luminance");

					auto inputR = makeInputFloat();
					inputR->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
					inputR->getDataAttributes().update<attr::InputData::limits>(math::fvec2 { 0.F, 255.F });
					inputR->getDataAttributes().setQuery<attr::InputData::value>(
						[component]() {
							return component->_luminance.r;
						}
					);

					auto inputG = makeInputFloat();
					inputG->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
					inputG->getDataAttributes().update<attr::InputData::limits>(math::fvec2 { 0.F, 255.F });
					inputG->getDataAttributes().setQuery<attr::InputData::value>(
						[component]() {
							return component->_luminance.g;
						}
					);

					auto inputB = makeInputFloat();
					inputB->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
					inputB->getDataAttributes().update<attr::InputData::limits>(math::fvec2 { 0.F, 255.F });
					inputB->getDataAttributes().setQuery<attr::InputData::value>(
						[component]() {
							return component->_luminance.b;
						}
					);

					grid->addChild(hg::move(label));
					for (auto&& item : { inputR, inputG, inputB }) {
						grid->addChild(hg::move(item));
					}

					list->addChild(hg::move(grid));
				}

				/**/

				{
					auto row = make_sptr<uikit::HorizontalLayout>();
					std::get<0>(row->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
					std::get<1>(row->getLayoutAttributes().attributeSets).update<attr::FlexLayout::justify>(
						ReflowSpacing::eSpaceBetween
					);

					auto label = make_sptr<uikit::Text>();
					label->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
					label->setText("Distance");

					auto input = makeInputFloat();
					input->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
					input->getDataAttributes().setQuery<attr::InputData::value>(
						[component]() {
							return component->_distanceLimit;
						}
					);

					row->addChild(hg::move(label));
					row->addChild(hg::move(input));

					list->addChild(hg::move(row));
				}

				/**/

				return list;
			}
		}
	);

	service_.registerProperty(
		{
			.debugName = "Skybox Component :: Geometry",
			.section = "Geometry",
			.selector = [](Property::selector_fn_arg data_) {
				return data_.is<nmpt<const HierarchyComponent>>() && data_.as<nmpt<const HierarchyComponent>>() != nullptr &&
					IsType<SkyboxComponent>(*data_.as<nmpt<const HierarchyComponent>>());
			},
			.view = [](Property::view_fn_arg data_) {
				const auto component = Cast<SkyboxComponent>(data_.as<nmpt<HierarchyComponent>>());

				auto list = make_sptr<uikit::VerticalLayout>();
				std::get<0>(list->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);

				/**/

				{
					auto row = make_sptr<uikit::HorizontalLayout>();
					std::get<0>(row->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
					std::get<1>(row->getLayoutAttributes().attributeSets).update<attr::FlexLayout::justify>(
						ReflowSpacing::eSpaceBetween
					);

					auto label = make_sptr<uikit::Text>();
					label->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
					label->setText("Geometry Asset");

					auto input = makeInputText();
					input->getDataAttributes().setQuery<attr::InputData::value>(
						[component]() {
							const auto& asset = component->getSkyboxGeometry();
							return asset.isValid() ? encodeGuid4228(asset.getAssetGuid()) : "";
						}
					);

					row->addChild(hg::move(label));
					row->addChild(hg::move(input));

					list->addChild(hg::move(row));
				}

				/**/

				{
					auto grid = make_sptr<uikit::UniformGridLayout>();

					std::get<0>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minHeight>(
						{ ReflowUnitType::eAbsolute, 40.F }
					);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxHeight>(
						{ ReflowUnitType::eAbsolute, 40.F }
					);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minWidth>(
						{ ReflowUnitType::eRelative, 1.F / 3.F }
					);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxWidth>(
						{ ReflowUnitType::eRelative, 1.F / 3.F }
					);

					auto label = make_sptr<uikit::Text>();
					label->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
					label->setText("Materials");

					auto slotLabel = make_sptr<uikit::Text>();
					slotLabel->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
					slotLabel->setText("Slot #1");

					auto input = makeInputText();
					input->getDataAttributes().setQuery<attr::InputData::value>(
						[component]() {
							const auto& asset = component->getSkyboxMaterial();
							return asset.isValid() ? encodeGuid4228(asset.getAssetGuid()) : "";
						}
					);

					grid->addChild(hg::move(label));
					grid->addChild(hg::move(slotLabel));
					grid->addChild(hg::move(input));

					list->addChild(hg::move(grid));
				}

				/**/

				return list;
			}
		}
	);

	service_.registerProperty(
		{
			.debugName = "Spot Light Component :: Lighting",
			.section = "Lighting",
			.selector = [](Property::selector_fn_arg data_) {
				return data_.is<nmpt<const HierarchyComponent>>() && data_.as<nmpt<const HierarchyComponent>>() != nullptr &&
					IsType<SpotLightComponent>(*data_.as<nmpt<const HierarchyComponent>>());
			},
			.view = [](Property::view_fn_arg data_) {
				const auto component = Cast<SpotLightComponent>(data_.as<nmpt<HierarchyComponent>>());

				auto list = make_sptr<uikit::VerticalLayout>();
				std::get<0>(list->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);

				/**/

				{
					auto grid = make_sptr<uikit::UniformGridLayout>();

					std::get<0>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minHeight>(
						{ ReflowUnitType::eAbsolute, 20.F }
					);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxHeight>(
						{ ReflowUnitType::eAbsolute, 20.F }
					);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minWidth>(
						{ ReflowUnitType::eRelative, 1.F / 4.F }
					);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxWidth>(
						{ ReflowUnitType::eRelative, 1.F / 4.F }
					);

					auto label = make_sptr<uikit::Text>();
					label->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
					label->setText("Luminance");

					auto inputR = makeInputFloat();
					inputR->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
					inputR->getDataAttributes().update<attr::InputData::limits>(math::fvec2 { 0.F, 255.F });
					inputR->getDataAttributes().setQuery<attr::InputData::value>(
						[component]() {
							return component->_luminance.r;
						}
					);

					auto inputG = makeInputFloat();
					inputG->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
					inputG->getDataAttributes().update<attr::InputData::limits>(math::fvec2 { 0.F, 255.F });
					inputG->getDataAttributes().setQuery<attr::InputData::value>(
						[component]() {
							return component->_luminance.g;
						}
					);

					auto inputB = makeInputFloat();
					inputB->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
					inputB->getDataAttributes().update<attr::InputData::limits>(math::fvec2 { 0.F, 255.F });
					inputB->getDataAttributes().setQuery<attr::InputData::value>(
						[component]() {
							return component->_luminance.b;
						}
					);

					grid->addChild(hg::move(label));
					for (auto&& item : { inputR, inputG, inputB }) {
						grid->addChild(hg::move(item));
					}

					list->addChild(hg::move(grid));
				}

				/**/

				{
					auto row = make_sptr<uikit::HorizontalLayout>();
					std::get<0>(row->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
					std::get<1>(row->getLayoutAttributes().attributeSets).update<attr::FlexLayout::justify>(
						ReflowSpacing::eSpaceBetween
					);

					auto label = make_sptr<uikit::Text>();
					label->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
					label->setText("Distance");

					auto input = makeInputFloat();
					input->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
					input->getDataAttributes().setQuery<attr::InputData::value>(
						[component]() {
							return component->_distanceLimit;
						}
					);

					row->addChild(hg::move(label));
					row->addChild(hg::move(input));

					list->addChild(hg::move(row));
				}

				/**/

				{
					auto row = make_sptr<uikit::HorizontalLayout>();
					std::get<0>(row->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
					std::get<1>(row->getLayoutAttributes().attributeSets).update<attr::FlexLayout::justify>(
						ReflowSpacing::eSpaceBetween
					);

					auto label = make_sptr<uikit::Text>();
					label->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
					label->setText("Inner Cone");

					auto input = makeInputFloat();
					input->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
					input->getDataAttributes().setQuery<attr::InputData::value>(
						[component]() {
							return component->_innerCone;
						}
					);

					row->addChild(hg::move(label));
					row->addChild(hg::move(input));

					list->addChild(hg::move(row));
				}

				/**/

				{
					auto row = make_sptr<uikit::HorizontalLayout>();
					std::get<0>(row->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
					std::get<1>(row->getLayoutAttributes().attributeSets).update<attr::FlexLayout::justify>(
						ReflowSpacing::eSpaceBetween
					);

					auto label = make_sptr<uikit::Text>();
					label->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
					label->setText("Outer Cone");

					auto input = makeInputFloat();
					input->getContent()->getLayoutAttributes().update<attr::BoxLayout::widthGrow>(1.F);
					input->getDataAttributes().setQuery<attr::InputData::value>(
						[component]() {
							return component->_outerCone;
						}
					);

					row->addChild(hg::move(label));
					row->addChild(hg::move(input));

					list->addChild(hg::move(row));
				}

				/**/

				return list;
			}
		}
	);

	service_.registerProperty(
		{
			.debugName = "Static Geometry Component",
			.section = "Geometry",
			.selector = [](Property::selector_fn_arg data_) {
				return data_.is<nmpt<const HierarchyComponent>>() && data_.as<nmpt<const HierarchyComponent>>() != nullptr &&
					IsType<StaticGeometryComponent>(*data_.as<nmpt<const HierarchyComponent>>());
			},
			.view = [](Property::view_fn_arg data_) {
				const auto component = Cast<StaticGeometryComponent>(data_.as<nmpt<HierarchyComponent>>());

				auto list = make_sptr<uikit::VerticalLayout>();
				std::get<0>(list->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);

				/**/

				{
					auto row = make_sptr<uikit::HorizontalLayout>();
					std::get<0>(row->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
					std::get<1>(row->getLayoutAttributes().attributeSets).update<attr::FlexLayout::justify>(
						ReflowSpacing::eSpaceBetween
					);

					auto label = make_sptr<uikit::Text>();
					label->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
					label->setText("Geometry Asset");

					/*
					auto input = make_sptr<uikit::Text>();
					input->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
					input->setText(
						component->getStaticGeometry().getAssetRef<>().map_or(
							[](const auto& asset_) {
								return asset_.getAssetName();
							},
							encodeGuid4228(component->getStaticGeometry().getAssetGuid())
						)
					);
					*/

					auto input = makeInputSelect();
					input->getDataAttributes().setQuery<attr::InputData::value>(
						[component]() {
							return component->getStaticGeometry().getAssetRef<>().map_or_else(
								[](const auto& asset_) {
									return asset_.getAssetName();
								},
								[component]() {
									return encodeGuid4228(component->getStaticGeometry().getAssetGuid());
								}
							);
						}
					);
					::hg::discard = input->onSelectMenu(
						[component, index = 0uL](ref<const SelectMenuEvent> event_) {

							auto& builder = event_.builder;
							const auto& assetRegistry = static_cast<ref<const assets::AssetRegistry>>(
								*Engine::getEngine()->getAssets()->getRegistry()
							);

							auto geometryAssets = Vector<nmpt<assets::Asset>> {};
							assetRegistry.findAssetsByType(::hg::refl::TypeId<assets::StaticGeometry>(), geometryAssets);

							/**/

							for (const auto& geometry : geometryAssets) {

								const auto name = geometry->getAssetName();
								const auto& url = geometry->getAssetStorageUrl();

								auto item = builder.addItem(encodeGuid4228(geometry->getAssetGuid()));
								item.setTitle(name.empty() ? url.encode() : name);

								item.setAction(
									[component, geometryGuid = geometry->getAssetGuid()] {

										const auto result = Heliogrim::assets().find<StaticGeometryAssetHandle>(geometryGuid);
										component->setStaticGeometry(result.value);
										return true;
									}
								);

							}

							return EventResponse::eHandled;
						}
					);

					row->addChild(hg::move(label));
					row->addChild(hg::move(input));

					list->addChild(hg::move(row));
				}

				/**/

				{
					auto grid = make_sptr<uikit::UniformGridLayout>();

					std::get<0>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::widthGrow>(1.F);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minHeight>(
						{ ReflowUnitType::eAbsolute, 40.F }
					);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxHeight>(
						{ ReflowUnitType::eAbsolute, 40.F }
					);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::minWidth>(
						{ ReflowUnitType::eRelative, 1.F / 3.F }
					);
					std::get<2>(grid->getLayoutAttributes().attributeSets).update<attr::BoxLayout::maxWidth>(
						{ ReflowUnitType::eRelative, 1.F / 3.F }
					);

					auto label = make_sptr<uikit::Text>();
					label->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
					label->setText("Materials");

					grid->addChild(hg::move(label));

					const auto& asset = component->getStaticGeometry();
					const auto& materials = component->getInstanceMaterials();

					if (asset.isValid() && asset.getAssetRef()->getMeshMaterialCount() > 0uL) {

						auto slotLabel = make_sptr<uikit::Text>();
						slotLabel->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
						slotLabel->setText("Slot #1");

						auto input = makeInputSelect();
						input->getDataAttributes().setQuery<attr::InputData::value>(
							[component, index = 0uL]() {

								const auto& materials = component->getInstanceMaterials();
								if (materials.size() < index || materials[index] == None || not materials[index]->isValid()) {
									return String { "Unset" };
								}

								const auto& material = materials[index].value();
								return material.getAssetRef<>().map_or_else(
									[](const auto& asset_) {
										return asset_.getAssetName();
									},
									[&material]() {
										return encodeGuid4228(material.getAssetGuid());
									}
								);
							}
						);
						::hg::discard = input->onSelectMenu(
							[component, index = 0uL](ref<const SelectMenuEvent> event_) {

								auto& builder = event_.builder;
								const auto& assetRegistry = static_cast<ref<const assets::AssetRegistry>>(
									*Engine::getEngine()->getAssets()->getRegistry()
								);

								auto materialAssets = Vector<nmpt<assets::Asset>> {};
								assetRegistry.findAssetsByType(::hg::refl::TypeId<assets::GfxMaterial>(), materialAssets);

								auto prototypeAssets = Vector<nmpt<assets::Asset>> {};
								assetRegistry.findAssetsByType(::hg::refl::TypeId<assets::GfxMaterialPrototype>(), prototypeAssets);

								/**/

								for (const auto& material : materialAssets) {

									const auto name = material->getAssetName();
									const auto& url = material->getAssetStorageUrl();

									auto item = builder.addItem(encodeGuid4228(material->getAssetGuid()));
									item.setTitle(name.empty() ? url.encode() : name);

									item.setAction(
										[component, materialGuid = material->getAssetGuid(), index = 0uL] {

											const auto result = Heliogrim::assets().find<GfxMaterialAssetHandle>(materialGuid);
											component->setInstanceMaterial(index, result.value);
											return true;
										}
									);

								}

								return EventResponse::eHandled;
							}
						);

						//auto input = make_sptr<uikit::Text>();
						//input->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
						//input->setText(materials.size() > 0uL ? encodeGuid4228(materials.at(0uL)) : "");

						grid->addChild(hg::move(slotLabel));
						grid->addChild(hg::move(input));

						/**/

						for (u32 i = 1uL; i < asset.getAssetRef()->getMeshMaterialCount(); ++i) {

							slotLabel = make_sptr<uikit::Text>();
							slotLabel->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);
							slotLabel->setText(std::format(R"(Slot #{})", i));

							auto input = make_sptr<uikit::Text>();
							input->getStyleAttributes().update<attr::TextStyle::textAlign>(TextAlign::eMiddleLeft);

							const Opt<GfxMaterialAssetHandle>& material = materials.size() > i ? materials.at(i) : None;
							if (material != None && material->isValid()) {
								auto assetName = material->getAssetRef()->getAssetName();
								// TODO: input->setText(assetName.empty() ? encodeGuid4228(material->getAssetGuid()) : assetName);
								input->setText(assetName.empty() ? encodeGuid4228(material->getAssetGuid()) : "");
							}

							grid->addChild(NullWidget::instance());
							grid->addChild(hg::move(slotLabel));
							grid->addChild(hg::move(input));
						}
					}

					list->addChild(hg::move(grid));
				}

				/**/

				return list;
			}
		}
	);
}
