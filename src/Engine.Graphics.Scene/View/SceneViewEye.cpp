#include "SceneViewEye.hpp"

#include <Engine.Common/Math/Convertion.hpp>
#include <Engine.Common/Math/Coordinates.hpp>

using namespace hg::engine::gfx::scene;
using namespace hg;

SceneViewEye::SceneViewEye() :
	_location(),
	_projection(math::mat4::make_identity()),
	_view(math::mat4::make_identity()),
	_driver(SceneViewDriver::eVulkan) {}

void SceneViewEye::updateProjection(cref<math::mat4> projection_) {
	_projection = projection_;
	if (_driver == SceneViewDriver::eVulkan) {
		constexpr auto vkViewIntermediate = math::mat4 {
			1.F, 0.F, 0.F, 0.F,
			0.F, -1.F, 0.F, 0.F,
			0.F, 0.F, -1.F, 0.F,
			0.F, 0.F, 0.F, 1.F
		};
		_projection = _projection * vkViewIntermediate;
	}
}

void SceneViewEye::updateView(cref<math::Location> origin_, cref<math::Rotator> rotation_) {

	/* Handle Rotation */

	math::mat4 rotationMatrix { math::mat4::make_identity() };

	rotationMatrix.rotate(rotation_.pitch(), math::vec3_right);
	rotationMatrix.rotate(rotation_.yaw(), math::vec3_up);
	rotationMatrix.rotate(rotation_.roll(), math::vec3_forward);

	/* Handle Translation */

	_location = origin_;
	const auto translationMatrix = math::mat4::make_identity().translate(-origin_.into());

	/* Update View */

	constexpr bool fpc = true;// Using first person as default
	if constexpr (fpc) {
		_view = rotationMatrix * translationMatrix;
	} else {
		_view = translationMatrix * rotationMatrix;
	}

	_inverseView = _view.inverse();
}

void SceneViewEye::updateView(cref<math::Location> origin_, cref<math::quaternion> rotation_) {
	updateView(origin_, math::as<math::Rotator>(rotation_));
}

cref<math::Location> SceneViewEye::getOrigin() const noexcept {
	return _location;
}

cref<math::mat4> SceneViewEye::getProjectionMatrix() const noexcept {
	return _projection;
}

void SceneViewEye::getInvProjectionMatrix(ref<math::mat4> matrix_) const noexcept {
	matrix_ = _projection.inverse();
}

ref<const math::mat4> SceneViewEye::getViewMatrix() const noexcept {
	return _view;
}

ref<const math::mat4> SceneViewEye::getInvViewMatrix() const noexcept {
	return _inverseView;
}

math::vec4 SceneViewEye::universeToView(ref<const math::Location> universeLocation_) const {
	return _view * math::fvec4 { universeLocation_.into(), 1.F };
}

math::vec4 SceneViewEye::universeToViewProjected(ref<const math::Location> universeLocation_) const {
	return _projection * _view * math::fvec4 { universeLocation_.into(), 1.F };
}

math::vec4 SceneViewEye::viewToUniverse(ref<const math::Location> viewLocation_) const {
	return _inverseView * math::fvec4 { viewLocation_.into(), 1.F };
}

math::vec4 SceneViewEye::projectedViewToUniverse(ref<const math::Location> viewLocation_) const {

	auto ip = math::mat4::make_identity();
	getInvProjectionMatrix(ip);

	return _inverseView * ip * math::fvec4 { viewLocation_.into(), 1.F };
}

math::vec2 SceneViewEye::universeToScreen(ref<const math::Location> universeLocation_) const {

	// Affine projection of universe position to view space
	const math::vec4 viewSpace = universeToViewProjected(universeLocation_);
	if (viewSpace.w <= 0.F) {
		return math::vec2 { -1.F, -1.F };
	}

	// Normalize to screen-space
	const auto normFactor { 1.F / viewSpace.w };
	const math::vec4 hs { viewSpace.x * normFactor, viewSpace.y * normFactor, viewSpace.z * normFactor, viewSpace.w };

	// Move from screen-space to ui space
	const math::vec2 norm {
		(hs.x / 2.F) + 0.5F,
		/* 1.F - */
		(hs.y / 2.F) + 0.5F// No need to change y direction, as vulkan by default uses top-down space
	};

	return norm;
}

void SceneViewEye::screenToUniverse(
	cref<math::vec2> screenPosition_,
	ref<math::vec3> universeOrigin_,
	ref<math::vec3> universeDirection_
) const {
	const math::vec2 screen {
		(screenPosition_.x - 0.5F) * 2.F,
		(screenPosition_.y - 0.5F) * 2.F// @see vulkan specs at SceneViewEye::universeToScreen
	};

	const math::vec4 eyeToFar { screen.x, screen.y, 1.F, 1.F };
	const math::vec4 eyeToNear { screen.x, screen.y, 0.001F, 1.F };

	math::mat4 ip { math::mat4::make_identity() };
	getInvProjectionMatrix(ip);

	auto farH { ip * eyeToFar };
	auto nearH { ip * eyeToNear };

	if (farH.w != 0.F) {
		const auto normFactor { 1.F / farH.w };
		farH = { farH.x * normFactor, farH.y * normFactor, farH.z * normFactor, 1.F };
	}

	if (nearH.w != 0.F) {
		const auto normFactor { 1.F / nearH.w };
		nearH = { nearH.x * normFactor, nearH.y * normFactor, nearH.z * normFactor, 1.F };
	}

	/**/

	auto viewSpaceDir { farH - nearH };
	viewSpaceDir.normalize();

	/**/

	const auto origin { _inverseView * nearH };

	viewSpaceDir.w = 0.F;
	auto direction { _inverseView * viewSpaceDir };
	direction.normalize();

	/**/

	universeOrigin_ = origin;
	universeDirection_ = direction;
}
