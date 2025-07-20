#pragma once

#include <Engine.Common/Collection/AutoArray.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.GFX.Loader/Material/MaterialResource.hpp>
#include <Engine.GFX.Render.Subpass/Mesh/MeshCaptureInterface.hpp>
#include <Engine.Render.Scene/RenderSceneSystemModel.hpp>

namespace hg::engine::gfx::scene {
	class GeometryModel :
		public InheritMeta<GeometryModel, render::RenderSceneSystemModel> {
	public:
		using this_type = GeometryModel;

	protected:
		explicit GeometryModel(const ptr<SceneComponent> owner_);

	public:
		GeometryModel(mref<this_type>) noexcept = default;

		~GeometryModel() override = default;

	public:
		ref<this_type> operator=(mref<this_type>) noexcept = default;

	protected:
		AutoArray<Arc<MaterialResource>> _materials;

	public:
		[[nodiscard]] virtual Arc<MaterialResource> material(const u32 index_) const noexcept;

	public:
		virtual void capture(nmpt<render::MeshCaptureInterface> mci_) const noexcept = 0;
	};
}
