#pragma once
#include <Engine.Assets.Type/Geometry/StaticGeometry.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.GFX.Loader/Geometry/StaticGeometryResource.hpp>
#include <Engine.Resource/Manage/Resource.hpp>

#include "GeometryModel.hpp"

namespace hg::engine::gfx::scene {
	class StaticGeometryModel final :
		public InheritMeta<StaticGeometryModel, GeometryModel> {
	public:
		using this_type = StaticGeometryModel;

		inline constexpr static type_id typeId { "StaticGeometryModel"_typeId };

	public:
		StaticGeometryModel(const ptr<SceneComponent> owner_);

		StaticGeometryModel(mref<this_type>) noexcept = default;

		~StaticGeometryModel() override;

	public:
		ref<this_type> operator=(mref<this_type>) noexcept = default;

	private:
		void tidy();

	public:
		void create(const ptr<render::RenderSceneSystem> system_) override;

		void update(const ptr<render::RenderSceneSystem> system_) override;

		void destroy(const ptr<render::RenderSceneSystem> system_) override;

	public:
		void capture(nmpt<render::MeshCaptureInterface> mci_) const noexcept override;

	private:
	public:
		u32 _sceneInstanceIndex;

	private:
		ptr<assets::StaticGeometry> _staticGeometryAsset = nullptr;
		smr<StaticGeometryResource> _staticGeometryResource;

	public:
		[[nodiscard]] const ptr<assets::StaticGeometry> geometryAsset() const noexcept;

		[[nodiscard]] cref<smr<StaticGeometryResource>> geometryResource() const noexcept;
	};
}
