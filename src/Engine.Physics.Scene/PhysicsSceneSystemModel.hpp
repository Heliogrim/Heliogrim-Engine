#pragma once

#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

namespace hg {
	class SceneComponent;
}

namespace hg::engine::physics {
	class PhysicsSceneSystem;
}

namespace hg::engine::physics {
	class macro_novtable PhysicsSceneSystemModel :
		public InheritBase<PhysicsSceneSystemModel> {
	public:
		using this_type = PhysicsSceneSystemModel;

	protected:
		PhysicsSceneSystemModel() noexcept = default;

		explicit PhysicsSceneSystemModel(ref<SceneComponent> owner_) noexcept;

	public:
		PhysicsSceneSystemModel(this_type&&) noexcept = default;

		PhysicsSceneSystemModel(ref<const this_type>) = delete;

		~PhysicsSceneSystemModel() noexcept override = default;

	public:
		ref<this_type> operator=(mref<this_type>) noexcept = default;

		ref<this_type> operator=(ref<const this_type>) = delete;

	protected:
		nmpt<SceneComponent> _owner;

	public:
		[[nodiscard]] const nmpt<SceneComponent> owner() const noexcept;

	public:
		virtual void create(ref<PhysicsSceneSystem> system_) = 0;

		virtual void update(ref<PhysicsSceneSystem> system_) = 0;

		virtual void readback(ref<PhysicsSceneSystem> system_) = 0;

		virtual void destroy(ref<PhysicsSceneSystem> system_) = 0;
	};
}
