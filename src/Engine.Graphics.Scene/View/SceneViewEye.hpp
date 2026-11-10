#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Math/Location.hpp>
#include <Engine.Common/Math/Matrix.hpp>
#include <Engine.Common/Math/Quaternion.hpp>
#include <Engine.Common/Math/Rotator.hpp>

namespace hg::engine::gfx::scene {
	enum class SceneViewDriver : bool {
		eNone,
		eVulkan
	};

	class SceneViewEye {
	public:
		using this_type = SceneViewEye;

	public:
		SceneViewEye();

	protected:
		math::Location _location;

		math::mat4 _projection;
		math::mat4 _view;
		math::mat4 _inverseView;

	protected:
		SceneViewDriver _driver;

	public:
		void updateProjection(cref<math::mat4> projection_);

		void updateView(cref<math::Location> origin_, cref<math::Rotator> rotation_);

		void updateView(cref<math::Location> origin_, cref<math::quaternion> rotation_);

	public:
		[[nodiscard]] cref<math::Location> getOrigin() const noexcept;

		[[nodiscard]] cref<math::mat4> getProjectionMatrix() const noexcept;

		void getInvProjectionMatrix(_Inout_ ref<math::mat4> matrix_) const noexcept;

		[[nodiscard]] ref<const math::mat4> getViewMatrix() const noexcept;

		[[nodiscard]] ref<const math::mat4> getInvViewMatrix() const noexcept;

	public:
		[[nodiscard]] math::vec4 universeToView(ref<const math::Location> universeLocation_) const;

		[[nodiscard]] math::vec4 universeToViewProjected(ref<const math::Location> universeLocation_) const;

		[[nodiscard]] math::vec4 viewToUniverse(ref<const math::Location> viewLocation_) const;

		[[nodiscard]] math::vec4 projectedViewToUniverse(ref<const math::Location> viewLocation_) const;

	public:
		/**
		 * Calculate the projected universe space position into ui space
		 *
		 * @author Julius
		 * @date 03.12.2022
		 *
		 * @param universeLocation_ The universe space position to project into ui space
		 *
		 * @returns A vector in normalized space `[0.0 ... 1.0]` when  succeeded, otherwise {-1, -1}
		 */
		[[nodiscard]] math::vec2 universeToScreen(ref<const math::Location> universeLocation_) const;

		/**
		 * Calculate the trace origin and direction un-projected from ui position to universe space
		 *
		 * @author Julius
		 * @date 03.12.2022
		 *
		 * @param screenPosition_ The ui space position in normalized space `[0.0 ... 1.0]`
		 * @param [Out] universeOrigin_ The universe space origin position for the trace
		 * @param [Out] universeDirection_ The universe space vector direction for the trace
		 */
		void screenToUniverse(
			cref<math::vec2> screenPosition_,
			_Out_ ref<math::vec3> universeOrigin_,
			_Out_ ref<math::vec3> universeDirection_
		) const;

	public:
		void setDriver(SceneViewDriver driver_);
	};
}
