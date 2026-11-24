#pragma once

#include "Capture/MeshCaptureInterface.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Graphics.Loader;
import Engine.Render.Scene;
#endif

namespace hg::engine::graphics::scene {
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
