#pragma once

#include <concepts>

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/AssociativeKey.hpp>
#include <Engine.Common/Collection/RobinMap.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include <Engine.GFX.Loader/Texture/TextureResource.hpp>
#include <Engine.GFX.Loader/Geometry/StaticGeometryResource.hpp>

#include "CacheCtrlSubject.hpp"
#include "CacheStaticGeometrySubject.hpp"
#include "CacheTextureSubject.hpp"
#include "__fwd.hpp"

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
        RobinMap<ptr<TextureResource>, RobinMap<AssocKey<TextureSubResource>, ptr<CacheCtrlSubject<
            TextureSubResource>>>> _textures;

    private:
        void dispatchLoad(const ptr<TextureResource> resource_, cref<TextureSubResource> subresource_);

        void dispatchUnload(const ptr<TextureResource> resource_, cref<TextureSubResource> subresource_);

    public:
        void markAsUsed(ptr<TextureResource> resource_, mref<TextureSubResource> subresource_);

        void markAsUsed(ptr<TextureResource> resource_, cref<AssocKey<TextureSubResource>> subresource_);

        [[deprecated]] void markAsUsed(ptr<TextureResource>, mref<TextureSubResourceRange> subresourceRange_);

        void unmark(ptr<TextureResource> resource_, mref<TextureSubResource> subresource_);

        void unmark(ptr<TextureResource> resource_, cref<AssocKey<TextureSubResource>> subresource_);

        [[deprecated]] void unmark(ptr<TextureResource>, mref<TextureSubResourceRange> subresourceRange_);

    private:
        /**
         * Static Geometries
         */
        RobinMap<ptr<StaticGeometryResource>, Vector<ptr<CacheCtrlSubject<StaticGeometrySubResource>>>>
        _staticGeometries;

    private:
        [[deprecated]] void dispatchLoad(
            const ptr<StaticGeometryResource> resource_,
            cref<StaticGeometrySubResource> subresource_
        );

        [[deprecated]] void dispatchUnload(
            const ptr<StaticGeometryResource> resource_,
            cref<StaticGeometrySubResource> subresource_
        );

    public:
        [[deprecated]] void markAsUsed(
            ptr<StaticGeometryResource> resource_,
            mref<StaticGeometrySubResource> subresource_
        );

        [[deprecated]] void unmark(ptr<StaticGeometryResource> resource_, mref<StaticGeometrySubResource> subresource_);
    };
}
