#pragma once

#if defined(HG_MODULE_INTERFACE_UNIT)
// Module aggregation includes this header before Location/Matrix/Rotator; Transform.hpp needs them first.
#include "Location.hpp"
#include "Matrix.hpp"
#include "Quaternion.hpp"
#include "Rotator.hpp"
#include "Vector.hpp"
#endif

#include <Engine.Common/Math/__default.inl>

namespace hg {
	using Color3 = math::fvec3;
	using Color4 = math::fvec4;

	using Mat2 = math::fmat2;
	using Mat3 = math::fmat3;
	using Mat4 = math::fmat4;

	using Quatern = math::fquaternion;

	using Transform = math::Transform;

	using Vector2 = math::fvec2;
	using Vector3 = math::fvec3;
	using Vector4 = math::fvec4;
}
