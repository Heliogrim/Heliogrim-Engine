#pragma once

#include <Engine.Graphics.Loader/Geometry/StaticGeometryResource.hpp>
#include <Engine.Graphics.Scene.Model/GeometryModel.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Assets.Type;
import Engine.Resource;
#endif

namespace hg {
	class StaticGeometryComponent;
}

namespace hg::engine::graphics {
	class SkeletalGeometryModel :
		public InheritMeta<StaticGeometryComponent, scene::GeometryModel> {
	public:
		SkeletalGeometryModel(const ptr<SceneComponent> owner_);

		~SkeletalGeometryModel() override;

	public:
		void create(const ptr<render::RenderSceneSystem> system_) override;

		void update(const ptr<render::RenderSceneSystem> system_) override;

		void destroy(const ptr<render::RenderSceneSystem> system_) override;

	public:
		void capture(nmpt<render::MeshCaptureInterface> mci_) const noexcept override;

	private:
		ptr<assets::SkeletalGeometry> _skeletalGeometry = nullptr;
		// TODO:
		ptr<StaticGeometryResource> _skeletalGeometryResource = nullptr;
		bool _streamable = false;
	};
}
