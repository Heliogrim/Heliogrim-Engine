#pragma once

#include <Engine.Accel.Compile/EffectCompileResult.hpp>
#include <Engine.Accel.Effect/AccelerationEffect.hpp>
#include <Engine.Accel.Pass/GraphicsPass.hpp>
#include <Engine.Graphics/Framebuffer/Framebuffer.hpp>
#include <Engine.Graphics.RenderGraph/Symbol/Subscribed.hpp>
#include <Engine.Graphics.RenderGraph/Symbol/SymbolizedResource.hpp>
#include <Engine.Graphics/Buffer/Buffer.hpp>
#include <Engine.Graphics/Buffer/StorageBufferView.hpp>
#include <Engine.Graphics/Buffer/UniformBufferView.hpp>
#include <Engine.Graphics/Texture/TextureSampler.hpp>
#include <Engine.Graphics/Texture/TextureView.hpp>
#include <Engine.Graphics.Scene.Model/StaticGeometryModel.hpp>

#include "../Mesh/MeshSubPass.hpp"

namespace hg::engine::render {
	class MatTestPass :
		public MeshSubPass {
	private:
		struct Payload {
			gfx::material::MaterialEffect effect;
			accel::EffectCompileResult compiled;
			Vector<smr<engine::gfx::MaterialResource>> materials;
			Vector<ptr<const gfx::scene::StaticGeometryModel>> models;
		};

		DenseMap<smr<const accel::AccelerationEffect>, Payload> _batches;

		smr<const accel::GraphicsPass> _pass;
		uptr<gfx::TextureSampler> _sampler;
		uptr<gfx::TextureSampler> _depthSampler;

		uptr<gfx::StorageBufferView> _staticInstanceView;
		uptr<gfx::UniformBufferView> _sceneLightInfoView;
		uptr<gfx::StorageBufferView> _sceneLightView;
		uptr<gfx::StorageBufferView> _sceneShadowView;

		gfx::Buffer _cameraBuffer;
		uptr<gfx::UniformBufferView> _cameraBufferView;

		uptr<gfx::TextureView> _brdfLutView;
		uptr<gfx::TextureView> _brdfPrefView;
		uptr<gfx::TextureView> _brdfIrradView;
		smr<gfx::Framebuffer> _framebuffer;
		vk::Semaphore _tmpSignal;

		uptr<gfx::TextureView> _shadowDirView;

		struct Resources {
			graph::Subscribed<graph::SymbolizedResource> inOutSceneColor;
			graph::Subscribed<graph::SymbolizedResource> inOutDepthBuffer;
		} _resources;

	public:
		void destroy() noexcept override;

	public:
		void declareTransforms(ref<graph::ScopedSymbolContext> symCtx_) noexcept override;

	public:
		void iterate(cref<graph::ScopedSymbolContext> symCtx_) noexcept override;

		void resolve() noexcept override;

		void execute(cref<graph::ScopedSymbolContext> symCtx_) noexcept override;
	};
}
