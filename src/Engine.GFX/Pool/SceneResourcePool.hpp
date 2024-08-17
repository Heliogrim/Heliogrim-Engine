#pragma once
#include <Engine.Common/Math/Matrix.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Scene/Scene.hpp>

#include "InstancePooled.hpp"
#include "../Aabb.hpp"
#include "../Device/Device.hpp"
#include "../Light/Light.hpp"
#include "../Light/Shadow.hpp"

namespace hg::engine::render {
	class RenderSceneSystem;
}

namespace hg::engine::gfx {
	class SceneResourcePool {
	public:
		using this_type = SceneResourcePool;

	public:
		SceneResourcePool(cref<sptr<Device>> device_);

		SceneResourcePool(cref<this_type>) = delete;

		SceneResourcePool(mref<this_type>) = delete;

		~SceneResourcePool();

	public:
		void setup();

		void destroy();

	private:
		sptr<Device> _device;
		nmpt<render::RenderSceneSystem> _system;

	public:
		[[nodiscard]] sptr<Device> device() const noexcept;

		[[nodiscard]] nmpt<render::RenderSceneSystem> system() const noexcept;

	private:
	public:
		InstancePooled<math::mat4> staticInstancePool;
		InstancePooled<Aabb3d> staticAabbPool;

	private:
	public:
		GlslSceneLightInfo sceneLightInfo;

		TypeBuffer<GlslSceneLightInfo> sceneLightInfoBuffer;
		InstancePooled<GlslLight> lightSourcePool;

		GlslDirectionalShadowInfo sceneShadowInfo;

		TypeBuffer<GlslDirectionalShadowInfo> sceneShadowInfoBuffer;
		InstancePooled<GlslDirectionalShadow> shadowSourcePool;
	};
}
