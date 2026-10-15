#pragma once

#include "jolt.inc.hpp"

/**/

#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/Array.hpp>
#include <Engine.Common/Meta/IsAnyOf.hpp>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

#include "PhysicsLayer.hpp"

namespace hg::engine::physics {
	struct ObjectLayerPairFilterImpl : JPH::ObjectLayerPairFilter {
		[[nodiscard]] bool ShouldCollide(const JPH::ObjectLayer lhs_, const JPH::ObjectLayer rhs_) const noexcept override {
			switch (lhs_) {
				case PhysicsLayer::eStatic: { return rhs_ == PhysicsLayer::eMoving; }
				case PhysicsLayer::eMoving: return true;
				default: return false;
			}
		}
	};

	namespace BroadPhaseLayer {
		constexpr static auto eStatic = JPH::BroadPhaseLayer { 0 };
		constexpr static auto eMoving = JPH::BroadPhaseLayer { 1 };
	}

	struct BroadPhaseLayerImpl : JPH::BroadPhaseLayerInterface {
		BroadPhaseLayerImpl() :
			objectToBroadPhaseMapping {} {
			objectToBroadPhaseMapping[PhysicsLayer::eStatic] = BroadPhaseLayer::eStatic;
			objectToBroadPhaseMapping[PhysicsLayer::eMoving] = BroadPhaseLayer::eMoving;
		};

		[[nodiscard]] JPH::uint GetNumBroadPhaseLayers() const noexcept override {
			return objectToBroadPhaseMapping.size();
		}

		[[nodiscard]] JPH::BroadPhaseLayer GetBroadPhaseLayer(const JPH::ObjectLayer layer_) const override {
			::hg::assertd(layer_ < 2u);
			return objectToBroadPhaseMapping[layer_];
		}

		[[nodiscard]] const char* GetBroadPhaseLayerName(const JPH::BroadPhaseLayer layer_) const override {
			switch (static_cast<JPH::BroadPhaseLayer::Type>(layer_)) {
				case static_cast<JPH::BroadPhaseLayer::Type>(BroadPhaseLayer::eStatic): return "Static";
				case static_cast<JPH::BroadPhaseLayer::Type>(BroadPhaseLayer::eMoving): return "Moving";
				default: {
					::hg::assertd(false);
					return "Unknown";
				}
			}
		}

		Array<JPH::BroadPhaseLayer, 2> objectToBroadPhaseMapping;
	};

	struct ObjectVsBroadPhaseLayerFilterImpl : JPH::ObjectVsBroadPhaseLayerFilter {
		[[nodiscard]] bool ShouldCollide(const JPH::ObjectLayer objectLayer_, const JPH::BroadPhaseLayer broadPhaseLayer_) const override {
			switch (objectLayer_) {
				case PhysicsLayer::eStatic: return broadPhaseLayer_ == BroadPhaseLayer::eMoving;
				case PhysicsLayer::eMoving: return true;
				default: {
					::hg::assertd(false);
					return false;
				}
			}
		}
	};

	template <typename... Interfaces_>
	struct PhysicsSystemHooks :
		public Interfaces_... {
		template <IsAnyOf<Interfaces_...> Interface_>
		[[nodiscard]] ref<Interface_> getInterface() noexcept {
			return static_cast<ref<Interface_>>(*this);
		}
	};

	class JphPhysicsSystemHooks :
		public PhysicsSystemHooks<ObjectLayerPairFilterImpl, BroadPhaseLayerImpl, ObjectVsBroadPhaseLayerFilterImpl> {
	public:
		using PhysicsSystemHooks::PhysicsSystemHooks;
	};
}
