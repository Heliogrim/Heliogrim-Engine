#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "__fwd.hpp"
#include "../Buffer/SparseBuffer.hpp"
#include "../Device/Device.hpp"
#include "../Texture/SparseTexture.hpp"

/**/
#include "SparseRangePooled.hpp"
#include "../Texture/TextureFactory.hpp"

namespace hg::engine::gfx::pool {
	struct IndexBufferAllocation {
		u64 size;
	};

	struct VertexBufferAllocation {
		u64 size;
	};

	typedef SparseTextureBuildPayload SparseTextureAllocation;

	/**/

	class GlobalResourcePool final {
	public:
		using this_type = GlobalResourcePool;

	public:
		GlobalResourcePool(cref<sptr<Device>> device_);

		GlobalResourcePool(mref<this_type>) noexcept = delete;

		GlobalResourcePool(cref<this_type>) = delete;

		~GlobalResourcePool();

	public:
		void setup();

		void destroy();

	protected:
		sptr<Device> _device;

	public:
		[[nodiscard]] cref<sptr<Device>> device() const noexcept;

	private:
		/**
		 * Vertex Buffer
		 */
		SparseRangePooled<void> _globalVertexBuffer;
		Vector<uptr<SparseBuffer>> _vertexBuffers;

	private:
		/**
		 * Index Buffer
		 */
		SparseRangePooled<void> _globalIndexBuffer;
		Vector<uptr<SparseBuffer>> _indexBuffers;

	private:
		/**
		 * Texture Atlas
		 */
		u32 _atlasMaxLayers;
		math::uivec2 _atlasMaxLayerExtent;
		math::uivec2 _atlasMinLayerExtent;

		math::uivec2 _atlasMaxSpatialExtent;
		math::uivec2 _atlasMinSpatialExtent;

		Vector<uptr<SparseTexture>> _textureAtlas;

	public:
		[[nodiscard]] _Success_(return != nullptr) uptr<SparseBufferView> allocateIndexBuffer(
			_In_ mref<IndexBufferAllocation> allocation_
		);

		[[nodiscard]] _Success_(return != nullptr) uptr<SparseBufferView> allocateVertexBuffer(
			_In_ mref<VertexBufferAllocation> allocation_
		);

		[[nodiscard]] _Success_(return != nullptr) uptr<SparseTextureView> allocateSparseTexture(
			_In_ mref<SparseTextureAllocation> allocation_
		);
	};
}
