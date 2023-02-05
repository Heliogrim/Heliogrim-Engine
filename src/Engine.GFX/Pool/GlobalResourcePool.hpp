#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "__fwd.hpp"
#include "../Device/Device.hpp"
#include "../Buffer/VirtualBuffer.hpp"
#include "../Texture/VirtualTexture.hpp"

namespace ember::engine::gfx::pool {
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
        Vector<uptr<VirtualBuffer>> _vertexBuffers;

    private:
        /**
         * Index Buffer
         */
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

    private:
        /**
         * Material Buffer
         */
        Vector<_STD uint_fast16_t> _materialForward;
        Vector<uptr<VirtualBuffer>> _materialBuffer;

    public:
        [[nodiscard]] _Success_(return != nullptr) non_owning_rptr<VirtualBuffer> requestIndexBuffer(
            _In_ mref<Vector<u64>> pageSizes_
        );

        [[nodiscard]] _Success_(return != nullptr) non_owning_rptr<VirtualBuffer> requestVertexBuffer(
            _In_ mref<Vector<u64>> pageSizes_
        );

        [[nodiscard]] _Success_(return != nullptr) uptr<VirtualBufferView> requestVirtualBuffer(
            _In_ mref<non_owning_rptr<const void>> material_
        );

        [[nodiscard]] _Success_(return != nullptr) uptr<VirtualTextureView> requestVirtualTexture(
            _In_ mref<non_owning_rptr<const void>> texture_
        );
    };
}
