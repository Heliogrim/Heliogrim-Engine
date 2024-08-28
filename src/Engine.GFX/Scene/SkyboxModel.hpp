#pragma once

#include <Engine.Assets/Types/Geometry/StaticGeometry.hpp>
#include <Engine.GFX.Scene.Model/GeometryModel.hpp>
#include <Engine.Resource/Manage/Resource.hpp>

namespace hg::engine::gfx {
	class SkyboxModel final :
		public InheritMeta<SkyboxModel, scene::GeometryModel> {
	public:
		using this_type = SkyboxModel;

		inline constexpr static type_id typeId { "SkyboxModel"_typeId };

	public:
		SkyboxModel(const ptr<SceneComponent> owner_);

		SkyboxModel(mref<this_type>) noexcept = default;

		~SkyboxModel() override final;

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
		ptr<assets::StaticGeometry> _skyboxGeometryAsset = nullptr;
		smr<resource::ResourceBase> _skyboxGeometryResource = {};
		bool _streamable = false;

	public:
		[[nodiscard]] const ptr<assets::StaticGeometry> geometryAsset() const noexcept;

		[[nodiscard]] cref<smr<resource::ResourceBase>> geometryResource() const noexcept;

		[[nodiscard]] bool streamable() const noexcept;
	};
}
