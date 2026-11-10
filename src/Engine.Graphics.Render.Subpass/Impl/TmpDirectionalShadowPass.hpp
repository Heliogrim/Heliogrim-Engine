#pragma once

#include <Engine.Accel.Compile/EffectCompileResult.hpp>
#include <Engine.Accel.Pass/GraphicsPass.hpp>
#include <Engine.Graphics.RenderGraph/Symbol/Observed.hpp>
#include <Engine.Graphics.RenderGraph/Symbol/Subscribed.hpp>
#include <Engine.Graphics/Framebuffer/Framebuffer.hpp>
#include <Engine.Graphics/Texture/Texture.hpp>
#include <Engine.Graphics.Scene.Model/DirectionalLightModel.hpp>
#include <Engine.Graphics.Scene.Model/StaticGeometryModel.hpp>
#include <Engine.Graphics.RenderGraph/Symbol/SymbolizedResource.hpp>
#include <Engine.Graphics/Buffer/Buffer.hpp>
#include <Engine.Graphics/Buffer/StorageBufferView.hpp>
#include <Engine.Graphics/Buffer/UniformBufferView.hpp>

#include "../Mesh/MeshSubPass.hpp"

namespace hg::engine::render {
	class TmpDirectionalShadowPass :
		public MeshSubPass {
	public:
		using this_type = TmpDirectionalShadowPass;

	public:
		TmpDirectionalShadowPass();

		~TmpDirectionalShadowPass() override;

	private:
		smr<graphics::material::MaterialEffect> _effect;
		smr<const accel::GraphicsPass> _pass;
		accel::EffectCompileResult _compiled;
		vk::Semaphore _tmpSemaphore;

		uptr<graphics::StorageBufferView> _staticInstanceView;
		uptr<graphics::StorageBufferView> _sceneLightView;
		uptr<graphics::StorageBufferView> _sceneShadowView;

		nmpt<const graphics::scene::DirectionalLightModel> _dirLightModel;
		smr<graphics::Texture> _dirShadowMap;
		smr<graphics::TextureView> _dirShadowCascades[4];
		smr<graphics::Framebuffer> _framebuffer[4];

		struct Resources {
			graph::Subscribed<graph::SymbolizedResource> inSceneView;
			graph::Observed<graph::SymbolizedResource> outDirShadows;
		} _resources;

	public:
		void setup(ref<graph::ScopedSymbolContext> symCtx_) noexcept override;

		void destroy() noexcept override;

		void declareOutputs(ref<graph::ScopedSymbolContext> symCtx_) noexcept override;

		void declareInputs(ref<graph::ScopedSymbolContext> symCtx_) noexcept override;

	public:
		void iterate(cref<graph::ScopedSymbolContext> symCtx_) noexcept override;

		void resolve() noexcept override;

		void execute(cref<graph::ScopedSymbolContext> symCtx_) noexcept override;
	};
}
