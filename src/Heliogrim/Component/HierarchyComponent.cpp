#include "HierarchyComponent.hpp"

#include <array>
#include <Engine.Common/Move.hpp>
#include <Engine.Common/Collection/InlineAutoArray.hpp>
#include <Engine.Common/Math/Quaternion.hpp>

using namespace hg;

HierarchyComponent::HierarchyComponent(
	mref<ComponentGuid> guid_,
	mref<ComponentTypeId> typeId_,
	mref<CachedActorPointer> owner_,
	mref<ptr<HierarchyComponent>> parent_
) :
	InheritMeta(::hg::move(guid_), ::hg::move(typeId_)),
	_owner(::hg::move(owner_)),
	_parent(::hg::move(parent_)),
	_localTransform({}, {}, math::vec3 { 1.F }) {}

HierarchyComponent::HierarchyComponent(mref<ComponentGuid> guid_, mref<CachedActorPointer> owner_, mref<ptr<HierarchyComponent>> parent_) :
	InheritMeta(::hg::move(guid_), ComponentTypeId { typeId }),
	_owner(::hg::move(owner_)),
	_parent(::hg::move(parent_)),
	_localTransform({}, {}, math::vec3 { 1.F }) {}

ptr<Actor> HierarchyComponent::getOwner() const noexcept {
	return _owner.cached;
}

ptr<Actor> HierarchyComponent::getRootActor() const noexcept {
	const auto root { getRootComponent() };
	return root ? root->getOwner() : nullptr;
}

ptr<HierarchyComponent> HierarchyComponent::getRootComponent() const noexcept {

	if (not _parent) {
		return const_cast<ptr<HierarchyComponent>>(this);
	}

	return _parent->getRootComponent();
}

ptr<HierarchyComponent> HierarchyComponent::getParentComponent() const noexcept {
	return _parent;
}

cref<math::Bounding> HierarchyComponent::getBoundaries() const noexcept {
	return _boundaries;
}

cref<Transform> HierarchyComponent::getLocalTransform() const noexcept {
	return _localTransform;
}

ref<Transform> HierarchyComponent::getLocalTransform() noexcept {
	return _localTransform;
}

namespace {
	using Matrix3 = std::array<float, 9>;

	/**
	 * Accumulated transform from the root down to a component, stored as an affine map `x' = M * x + t`.
	 *
	 * `M` is row-major and contains rotation and scale, so non-uniform scale below a rotation is handled correctly.
	 * The rotation is tracked separately as quaternion, as it must stay free of scale.
	 */
	struct AccumulatedTransform {
		Matrix3 m;
		math::fvec3 t;
		math::quaternion q;
	};

	/**
	 * Multiplies two row-major 3x3 matrices `lhs * rhs`.
	 */
	[[nodiscard]] Matrix3 multiply(const Matrix3& lhs_, const Matrix3& rhs_) noexcept {
		return {
			lhs_[0] * rhs_[0] + lhs_[1] * rhs_[3] + lhs_[2] * rhs_[6],
			lhs_[0] * rhs_[1] + lhs_[1] * rhs_[4] + lhs_[2] * rhs_[7],
			lhs_[0] * rhs_[2] + lhs_[1] * rhs_[5] + lhs_[2] * rhs_[8],
			lhs_[3] * rhs_[0] + lhs_[4] * rhs_[3] + lhs_[5] * rhs_[6],
			lhs_[3] * rhs_[1] + lhs_[4] * rhs_[4] + lhs_[5] * rhs_[7],
			lhs_[3] * rhs_[2] + lhs_[4] * rhs_[5] + lhs_[5] * rhs_[8],
			lhs_[6] * rhs_[0] + lhs_[7] * rhs_[3] + lhs_[8] * rhs_[6],
			lhs_[6] * rhs_[1] + lhs_[7] * rhs_[4] + lhs_[8] * rhs_[7],
			lhs_[6] * rhs_[2] + lhs_[7] * rhs_[5] + lhs_[8] * rhs_[8]
		};
	}

	[[nodiscard]] math::fvec3 apply(const Matrix3& m_, const math::fvec3& v_) noexcept {
		return math::fvec3 {
			m_[0] * v_.x + m_[1] * v_.y + m_[2] * v_.z,
			m_[3] * v_.x + m_[4] * v_.y + m_[5] * v_.z,
			m_[6] * v_.x + m_[7] * v_.y + m_[8] * v_.z
		};
	}

	/**
	 * Inverts a row-major 3x3 matrix through its adjugate.
	 */
	[[nodiscard]] Matrix3 inverse(const Matrix3& m_) noexcept {

		const auto c0 = m_[4] * m_[8] - m_[5] * m_[7];
		const auto c1 = m_[5] * m_[6] - m_[3] * m_[8];
		const auto c2 = m_[3] * m_[7] - m_[4] * m_[6];
		const auto invDet = 1.F / (m_[0] * c0 + m_[1] * c1 + m_[2] * c2);

		return {
			c0 * invDet, (m_[2] * m_[7] - m_[1] * m_[8]) * invDet, (m_[1] * m_[5] - m_[2] * m_[4]) * invDet,
			c1 * invDet, (m_[0] * m_[8] - m_[2] * m_[6]) * invDet, (m_[2] * m_[3] - m_[0] * m_[5]) * invDet,
			c2 * invDet, (m_[1] * m_[6] - m_[0] * m_[7]) * invDet, (m_[0] * m_[4] - m_[1] * m_[3]) * invDet
		};
	}

	/**
	 * Local matrix `R * S` of a transform, where `S` scales the columns of the rotation.
	 */
	[[nodiscard]] Matrix3 rotationScale(const math::Transform& transform_) noexcept {
		auto m = transform_.rotator().asRotationMatrix();
		const auto& s = transform_.scale();
		for (auto row = 0uLL; row < 3uLL; ++row) {
			m[row * 3uLL + 0uLL] *= s.x;
			m[row * 3uLL + 1uLL] *= s.y;
			m[row * 3uLL + 2uLL] *= s.z;
		}
		return m;
	}

	/**
	 * Collects the whole parent chain once and folds it from the root down to the given component.
	 *
	 * Per level this costs one rotation matrix, one 3x3 product and one matrix-vector product, instead of
	 * building and multiplying full 4x4 matrices at every level (and repeating the walk per queried property).
	 */
	[[nodiscard]] AccumulatedTransform accumulate(const ptr<const HierarchyComponent> leaf_) noexcept {

		InlineAutoArray<ptr<const HierarchyComponent>, 8> chain {};
		for (auto cur = leaf_; cur != nullptr; cur = cur->getParentComponent()) {
			chain.emplace_back(cur);
		}

		/**/

		const auto& rootTransform = chain.back()->getLocalTransform();
		chain.pop_back();

		AccumulatedTransform acc {
			rotationScale(rootTransform),
			rootTransform.location().into(),
			rootTransform.rotator().into()
		};

		while (not chain.empty()) {

			const auto& local = chain.back()->getLocalTransform();
			chain.pop_back();

			const auto shift = apply(acc.m, local.location().into());
			acc.t = math::fvec3 { acc.t.x + shift.x, acc.t.y + shift.y, acc.t.z + shift.z };
			acc.m = multiply(acc.m, rotationScale(local));
			acc.q = acc.q * local.rotator().into();
		}

		return acc;
	}
}

HierarchyComponent::UniverseTransform HierarchyComponent::getUniverseTransform() const noexcept {

	const auto acc = accumulate(this);

	math::fmat4 matrix = math::fmat4::make_identity();
	for (auto row = 0uLL; row < 3uLL; ++row) {
		for (auto col = 0uLL; col < 3uLL; ++col) {
			matrix[col][row] = acc.m[row * 3uLL + col];
		}
	}
	matrix[3][0] = acc.t.x;
	matrix[3][1] = acc.t.y;
	matrix[3][2] = acc.t.z;

	return UniverseTransform {
		math::Location { acc.t },
		// A root keeps its authored angles; folding through a quaternion would normalize them.
		_parent ? math::Rotator { math::quaternion { acc.q } } : _localTransform.rotator(),
		::hg::move(matrix)
	};
}

void HierarchyComponent::setRelativeLocationAndRotator(
	mref<math::Location> outerLocation_,
	mref<math::Rotator> outerRotator_
) noexcept {
	if (not _parent) {
		_localTransform.setLocationAndRotator(::hg::move(outerLocation_), ::hg::move(outerRotator_));
		return;
	}

	const auto parent = accumulate(_parent);

	const auto delta = outerLocation_.into() - parent.t;
	const auto innerLocation = apply(inverse(parent.m), delta);

	math::quaternion innerQuat {};
	innerQuat._quat = glm::inverse(parent.q._quat) * outerRotator_.into()._quat;

	_localTransform.setLocationAndRotator(math::Location { innerLocation }, math::Rotator { ::hg::move(innerQuat) });
}

math::Location HierarchyComponent::getUniverseLocation() const noexcept {
	return getUniverseTransform().location;
}

math::Rotator HierarchyComponent::getUniverseRotator() const noexcept {
	return getUniverseTransform().rotator;
}

math::fmat4 HierarchyComponent::getUniverseMatrix() const noexcept {
	return getUniverseTransform().matrix;
}
