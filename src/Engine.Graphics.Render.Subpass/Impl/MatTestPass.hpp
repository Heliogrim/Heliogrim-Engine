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
			graphics::material::MaterialEffect effect;
			accel::EffectCompileResult compiled;
			Vector<smr<engine::graphics::MaterialResource>> materials;
			Vector<ptr<const graphics::scene::StaticGeometryModel>> models;
		};

		DenseMap<smr<const accel::AccelerationEffect>, Payload> _batches;

		smr<const accel::GraphicsPass> _pass;
		uptr<graphics::TextureSampler> _sampler;
		uptr<graphics::TextureSampler> _depthSampler;

		uptr<graphics::StorageBufferView> _staticInstanceView;
		uptr<graphics::UniformBufferView> _sceneLightInfoView;
		uptr<graphics::StorageBufferView> _sceneLightView;
		uptr<graphics::StorageBufferView> _sceneShadowView;

		graphics::Buffer _cameraBuffer;
		uptr<graphics::UniformBufferView> _cameraBufferView;

		uptr<graphics::TextureView> _brdfLutView;
		uptr<graphics::TextureView> _brdfPrefView;
		uptr<graphics::TextureView> _brdfIrradView;
		smr<graphics::Framebuffer> _framebuffer;
		vk::Semaphore _tmpSignal;

		uptr<graphics::TextureView> _shadowDirView;

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
