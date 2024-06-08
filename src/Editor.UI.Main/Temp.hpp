#pragma once

#include "EditorUI.hpp"
#include "Editor.UI/Modules/ObjectEditor.hpp"
#include "Editor.UI/Modules/SceneHierarchy.hpp"
#include "Editor.UI/Modules/SceneHierarchy/SceneViewEntry.hpp"
#include "Engine.Common/Make.hpp"
#include "Heliogrim/Actor.hpp"
#include "Heliogrim/StaticGeometryComponent.hpp"
#include "Heliogrim/Components/DirectionalLightComponent.hpp"
#include "Heliogrim/Components/PointLightComponent.hpp"

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

	inline void storeSceneHierarchyMeta(ref<EditorUI> editorUi_) {
		editorUi_.getSceneHierarchy()->storeGenerator(
			"sptr<SceneViewEntry>"_typeId,
			make_uptr<ui::HierarchyGenerator<sptr<ui::SceneViewEntry>>>()
		);
		editorUi_.getSceneHierarchy()->storeResolver(
			"sptr<SceneViewEntry>"_typeId,
			make_uptr<ui::HierarchyResolver<sptr<ui::SceneViewEntry>>>()
		);
	}
}
