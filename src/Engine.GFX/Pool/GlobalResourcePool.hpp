#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "__fwd.hpp"
#include "../Device/Device.hpp"
#include "../Buffer/VirtualBuffer.hpp"
#include "../Texture/VirtualTexture.hpp"

/**/
#include "RangePooled.hpp"
#include "../Texture/TextureFactory.hpp"

namespace hg::engine::gfx::pool {
    struct IndexBufferAllocation {
        u64 size;
    };

    struct VertexBufferAllocation {
        u64 size;
    };

    typedef VirtualTextureBuildPayload VirtualTextureAllocation;

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
        RangePooled<void> _globalVertexBuffer;
        Vector<uptr<VirtualBuffer>> _vertexBuffers;

    private:
        /**
         * Index Buffer
         */
        RangePooled<void> _globalIndexBuffer;
        Vector<uptr<VirtualBuffer>> _indexBuffers;

    private:
        /**
         * Texture Atlas
         */
        u32 _atlasMaxLayers;
        math::uivec2 _atlasMaxLayerExtent;
        math::uivec2 _atlasMinLayerExtent;

        math::uivec2 _atlasMaxSpatialExtent;
        math::uivec2 _atlasMinSpatialExtent;

        Vector<uptr<VirtualTexture>> _textureAtlas;

    public:
        [[nodiscard]] _Success_(return != nullptr) uptr<VirtualBufferView> allocateIndexBuffer(
            _In_ mref<IndexBufferAllocation> allocation_
        );

        [[nodiscard]] _Success_(return != nullptr) uptr<VirtualBufferView> allocateVertexBuffer(
            _In_ mref<VertexBufferAllocation> allocation_
        );

        [[nodiscard]] _Success_(return != nullptr) uptr<VirtualTextureView> allocateVirtualTexture(
            _In_ mref<VirtualTextureAllocation> allocation_
        );
    };
}
