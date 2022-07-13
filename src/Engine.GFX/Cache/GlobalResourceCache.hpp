#pragma once

#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Assets/Types/StaticGeometry.hpp>
#include <Engine.Assets/Types/Texture.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "__fwd.hpp"
#include "../Buffer/VirtualBuffer.hpp"
#include "../Device/Device.hpp"
#include "../Resource/StaticGeometryResource.hpp"
#include "../Resource/TextureResource.hpp"
#include "../Texture/VirtualTexture.hpp"

namespace ember::engine::gfx::cache {

    class GlobalResourceCache final {
    public:
        friend class GlobalCacheCtrl;

    public:
        using this_type = GlobalResourceCache;

    public:
        [[nodiscard]] static ptr<GlobalResourceCache> get() {
            return GlobalResourceCache::_instance;
        }

        static ptr<GlobalResourceCache> make(cref<sptr<Device>> device_);

        static void destroy();

    protected:
        /**
         * Singleton Instance
         */
        static ptr<this_type> _instance;

        /**
         * Constructor
         *
         * @author Julius
         * @date 18.06.2022
         *
         * @param   device_ The device.
         */
        GlobalResourceCache(cref<sptr<Device>> device_);

        /**
         * Destructor
         *
         * @author Julius
         * @date 18.06.2022
         */
        ~GlobalResourceCache();

    protected:
        sptr<Device> _device;

    public:
        [[nodiscard]] cref<sptr<Device>> device() const noexcept;

    public:
        [[nodiscard, deprecated]] bool hook(_In_ cref<asset_guid> guid_, _Inout_ ptr<StaticGeometryResource> resource_);

        [[nodiscard, deprecated]] bool unhook(_In_ cref<asset_guid> guid_,
            _Inout_ ptr<StaticGeometryResource> resource_);

    public:
        [[nodiscard]] ptr<StaticGeometryResource> request(const ptr<const assets::StaticGeometry> asset_);

        [[nodiscard]] ptr<TextureResource> request(const ptr<const assets::Texture> asset_);

        void drop(cref<asset_guid> guid_, mref<ptr<StaticGeometryResource>> resource_);

    private:
        /**
         *
         */
        ska::bytell_hash_map<asset_guid, ptr<res::Resource>> _mapped;

    private:
        /**
         * Vertex Buffer
         */
        Vector<uptr<VirtualBuffer>> _vertexBuffers;

    private:
        [[nodiscard]] non_owning_rptr<VirtualBuffer> requestVertexBuffer(mref<Vector<u64>> pageSizes_);

    private:
        /**
         * Index Buffer
         */
        Vector<uptr<VirtualBuffer>> _indexBuffers;

    private:
        [[nodiscard]] non_owning_rptr<VirtualBuffer> requestIndexBuffer(mref<Vector<u64>> pageSizes_);

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
    };
}
