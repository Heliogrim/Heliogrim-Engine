#pragma once

#include "EditorUI.hpp"
#include "Editor.UI/Modules/ObjectEditor.hpp"
#include "Engine.Common/Make.hpp"
#include "Heliogrim/Actor/Actor.hpp"
#include "Heliogrim/Component/Scene/Geometry/StaticGeometryComponent.hpp"
#include "Heliogrim/Component/Scene/Light/DirectionalLightComponent.hpp"
#include "Heliogrim/Component/Scene/Light/PointLightComponent.hpp"

namespace hg::editor {
	inline void storeActorMapping(ref<EditorUI> editorUi_) {
		editorUi_.getObjectEditor()->storeObjectMapper(
			"hg::Actor"_typeId,
			make_uptr<ui::ObjectValueMapper<::hg::Actor>>()
		);
		editorUi_.getObjectEditor()->storeObjectMapper(
			::hg::StaticGeometryComponent::typeId,
			make_uptr<ui::ObjectValueMapper<::hg::StaticGeometryComponent>>()
		);
		editorUi_.getObjectEditor()->storeObjectMapper(
			::hg::PointLightComponent::typeId,
			make_uptr<ui::ObjectValueMapper<::hg::PointLightComponent>>()
		);
		editorUi_.getObjectEditor()->storeObjectMapper(
			::hg::DirectionalLightComponent::typeId,
			make_uptr<ui::ObjectValueMapper<::hg::DirectionalLightComponent>>()
		);
	}
}
