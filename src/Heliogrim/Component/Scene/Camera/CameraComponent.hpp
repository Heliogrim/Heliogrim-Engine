#pragma once

#include <Engine.GFX/Camera/CameraModes.hpp>

#include "../../SceneComponent.hpp"

namespace hg {
	class CameraComponent :
		public InheritMeta<CameraComponent, SceneComponent> {
	public:
		using this_type = CameraComponent;
		using underlying_type = SceneComponent;

		using CameraProjectMode = ::hg::engine::gfx::CameraProjectionMode;
		using CameraViewMode = ::hg::engine::gfx::CameraViewMode;

		constexpr static component_type_id typeId { "CameraComponent"_typeId };

	public:
		CameraComponent(mref<CachedActorPointer> owner_, mref<ptr<HierarchyComponent>> parent_);

		~CameraComponent() override;

	private:
		float _fov;

	public:
		[[nodiscard]] float getFov() const noexcept;

		void setFov(float fov_);

	private:
		float _aspectRation;
		bool _lockedAspect;

	public:
		[[nodiscard]] float getAspectRatio() const noexcept;

		void setAspectRatio(float aspectRatio_);

		[[nodiscard]] bool isAspectLocked() const noexcept;

		void lockAspectRatio(bool lock_ = true);

	private:
		bool _visibleFrustum;

	public:
		[[nodiscard]] bool isFrustumVisible() const noexcept;

		void setFrustumVisible(bool visible_);

	private:
		math::vec3 _offset;

	public:
		[[nodiscard]] cref<math::vec3> offset() const noexcept;

		void setOffset(cref<math::vec3> offset_);

	private:
		CameraProjectMode _projectMode;
		CameraViewMode _viewMode;

	public:
		[[nodiscard]] CameraProjectMode getProjectMode() const noexcept;

		void setProjectMode(CameraProjectMode mode_);

		[[nodiscard]] CameraViewMode getViewMode() const noexcept;

		void setViewMode(CameraViewMode mode_);
	};
}
