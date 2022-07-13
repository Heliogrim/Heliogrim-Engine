#pragma once

#include <concepts>

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/RobinMap.hpp>

#include "__fwd.hpp"
#include "CacheCtrlSubject.hpp"
#include "CacheTextureSubject.hpp"
#include "CacheStaticGeometrySubject.hpp"
#include "../Resource/TextureResource.hpp"
#include "../Resource/StaticGeometryResource.hpp"

namespace ember::engine::gfx::cache {

    struct TextureSubResourceRange {
        math::uivec2 layers;
        math::uivec2 mips;

        math::uivec3 offset;
        math::uivec3 extent;
    };

    class GlobalCacheCtrl {
    public:
        using this_type = GlobalCacheCtrl;

    public:
        GlobalCacheCtrl(const ptr<GlobalResourceCache> cache_);

        GlobalCacheCtrl(cref<this_type>) = delete;

        GlobalCacheCtrl(mref<this_type>) noexcept = delete;

        ~GlobalCacheCtrl();

    public:
        ref<this_type> operator=(cref<this_type>) = delete;

        ref<this_type> operator=(mref<this_type>) noexcept = delete;

    private:
        const ptr<GlobalResourceCache> _cache;

    public:
        [[nodiscard]] const ptr<GlobalResourceCache> cache() const noexcept;

    private:
        /**
         * Texture Atlases
         */
        RobinMap<ptr<TextureResource>, Vector<ptr<CacheCtrlSubject<TextureSubResource>>>> _textures;

    public:
        void markAsUsed(ptr<TextureResource> resource_, mref<TextureSubResource> subresource_);

        [[deprecated]] void markAsUsed(ptr<TextureResource>, mref<TextureSubResourceRange> subresourceRange_);

        void unmark(ptr<TextureResource> resource_, mref<TextureSubResource> subresource_);

        [[deprecated]] void unmark(ptr<TextureResource>, mref<TextureSubResourceRange> subresourceRange_);

    private:
        /**
         * Static Geometries
         */
        RobinMap<ptr<StaticGeometryResource>, Vector<ptr<CacheCtrlSubject<StaticGeometrySubResource>>>>
        _staticGeometries;

    public:
        [[deprecated]] void markAsUsed(ptr<StaticGeometryResource> resource_,
            mref<StaticGeometrySubResource> subresource_);

        [[deprecated]] void unmark(ptr<StaticGeometryResource> resource_, mref<StaticGeometrySubResource> subresource_);
    };

}
