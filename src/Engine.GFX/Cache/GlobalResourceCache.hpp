#pragma once

#include <Engine.Assets/AssetGuid.hpp>
#include <Engine.Assets/Types/GfxMaterial.hpp>
#include <Engine.Assets/Types/Geometry/StaticGeometry.hpp>
#include <Engine.Assets/Types/Texture/Texture.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.GFX.Loader/Geometry/StaticGeometryResource.hpp>
#include <Engine.GFX.Loader/Material/MaterialResource.hpp>
#include <Engine.GFX.Loader/Texture/TextureResource.hpp>
#include <Engine.Resource/LoaderManager.hpp>

#include "__fwd.hpp"
#include "../Buffer/VirtualBuffer.hpp"
#include "../Device/Device.hpp"
#include "../Texture/VirtualTexture.hpp"

namespace ember::engine::gfx::cache {
    class GlobalResourceCache final {
    public:
        friend class GlobalCacheCtrl;
        friend class LocalResourceCache;

    public:
        using this_type = GlobalResourceCache;

        using cache_value_type = smr<resource::ResourceBase>;

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

    private:
        const non_owning_rptr<resource::LoaderManager> _loader;

    public:
        [[nodiscard, deprecated]] bool hook(
            _In_ cref<asset_guid> guid_,
            _In_ mref<smr<StaticGeometryResource>> resource_
        );

        [[nodiscard, deprecated]] bool unhook(
            _In_ cref<asset_guid> guid_,
            _Inout_ smr<StaticGeometryResource> resource_
        );

    public:
        [[nodiscard]] smr<StaticGeometryResource> request(const ptr<const assets::StaticGeometry> asset_);

        [[nodiscard]] smr<TextureResource> request(const ptr<const assets::Texture> asset_);

        [[nodiscard]] smr<MaterialResource> request(const ptr<const assets::GfxMaterial> asset_);

        void drop(cref<asset_guid> guid_, mref<smr<StaticGeometryResource>> resource_);

    private:
        /**
         *
         */
        ska::bytell_hash_map<asset_guid, smr<resource::ResourceBase>> _mapped;

    public:
        [[nodiscard]] bool contains(const non_owning_rptr<const assets::Asset> asset_) const noexcept;

        void store(_In_ cref<asset_guid> guid_, _In_ mref<smr<resource::ResourceBase>> resource_);

        bool remove(_In_ cref<asset_guid> guid_);

        bool remove(_In_ cref<asset_guid> guid_, _Out_ ref<smr<resource::ResourceBase>> resource_);

        void clear();

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
        /**
         * Material Buffer
         */
        Vector<_STD uint_fast16_t> _materialForward;
        Vector<uptr<VirtualBuffer>> _materialBuffer;
    };
}
