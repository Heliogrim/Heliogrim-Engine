#pragma once

#include <concepts>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/AssociativeKey.hpp>
#include <Engine.Common/Collection/RobinMap.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.GFX.Loader/Geometry/StaticGeometryResource.hpp>
#include <Engine.GFX.Loader/Texture/TextureResource.hpp>
#include <Engine.GFX.Loader/Material/MaterialResource.hpp>
#include <Engine.Resource/LoaderManager.hpp>
#include <Engine.GFX.Acc/__fwd.hpp>

#include "CacheResult.hpp"
#include "CacheCtrlSubject.hpp"
#include "CacheStaticGeometrySubject.hpp"
#include "CacheTextureSubject.hpp"
#include "__fwd.hpp"

namespace hg::engine::gfx::cache {
    struct TextureSubResourceRange {
        math::uivec2 layers;
        math::uivec2 mips;

        math::uivec3 offset;
        math::uivec3 extent;
    };

    class GlobalCacheCtrl {
    public:
        using this_type = GlobalCacheCtrl;

        template <typename Type_ = void>
        using stream_result_type = Result<StreamCacheResultType, Type_>;

        template <typename Type_ = void>
        using query_result_type = Result<QueryResultType, Type_>;

    public:
        GlobalCacheCtrl(mref<uptr<GlobalResourceCache>> cache_);

        GlobalCacheCtrl(cref<this_type>) = delete;

        GlobalCacheCtrl(mref<this_type>) noexcept = delete;

        ~GlobalCacheCtrl();

    public:
        ref<this_type> operator=(cref<this_type>) = delete;

        ref<this_type> operator=(mref<this_type>) noexcept = delete;

    private:
        uptr<GlobalResourceCache> _cache;
        const non_owning_rptr<const resource::LoaderManager> _loader;

    public:
        [[nodiscard]] const non_owning_rptr<GlobalResourceCache> cache() const noexcept;

        [[nodiscard]] const non_owning_rptr<const resource::LoaderManager> loader() const noexcept;

    private:
        /**
         * Texture Atlases
         */
        RobinMap<ptr<TextureResource>, RobinMap<AssocKey<TextureSubResource>, ptr<CacheCtrlSubject<
            TextureSubResource>>>> _textures;

    public:
        /**
         * Mark the sub-resource with usage marker to effect managed memory
         *
         * @details This function will implicitly load the targeted sub-resource definition if not present.
         *  Therefore this function guarantees a preserved mark as long as no error occurs while loading.
         *
         * @param resource_ The effected managed resource
         * @param subresource_ The sub-resource definition to preserve
         *
         * @returns A result of `eResidential` if main- and sub-resource got a cache hit, `eTransient` if currently in a volatile state, otherwise `eUndefined`
         */
        [[nodiscard]] stream_result_type<> markLoadedAsUsed(
            _In_ mref<smr<TextureResource>> resource_,
            _In_ mref<TextureSubResource> subresource_
        );

        [[nodiscard]] stream_result_type<> markLoadedAsUsed(
            _In_ mref<smr<TextureResource>> resource_,
            cref<AssocKey<TextureSubResource>> subresource_
        );

        [[nodiscard]] stream_result_type<> markLoadedAsUsed(
            _In_ mref<smr<TextureResource>> resource_,
            _In_ mref<TextureSubResourceRange> range_
        );

    public:
        /**
         * Mark the sub-resource with usage marker to effect managed memory
         *
         * @param resource_ The effected managed resource
         * @param subresource_ The sub-resource definition to preserve
         *
         * @returns A result of `eResidential` if main- and sub-resource got a cache hit, otherwise `eUndefined`
         */
        [[nodiscard]] stream_result_type<> markAsUsed(
            _In_ const non_owning_rptr<TextureResource> resource_,
            _In_ mref<TextureSubResource> subresource_
        );

        [[nodiscard]] stream_result_type<> markAsUsed(
            _In_ const non_owning_rptr<TextureResource> resource_,
            cref<AssocKey<TextureSubResource>> subresource_
        );

        [[deprecated]] stream_result_type<> markAsUsed(
            _In_ const non_owning_rptr<TextureResource> resource_,
            _In_ mref<TextureSubResourceRange> range_
        );

    public:
        void unmark(
            const non_owning_rptr<TextureResource> resource_,
            cref<AssocKey<TextureSubResource>> subresource_
        );

        void unmark(_In_ mref<smr<TextureResource>> resource_, mref<TextureSubResource> subresource_);

        void unmark(_In_ mref<smr<TextureResource>> resource_, cref<AssocKey<TextureSubResource>> subresource_);

        [[deprecated]] void unmark(_In_ mref<smr<TextureResource>>, mref<TextureSubResourceRange> subresourceRange_);

    private:
        /**
         * Static Geometries
         */
        RobinMap<ptr<StaticGeometryResource>, Vector<ptr<CacheCtrlSubject<StaticGeometrySubResource>>>>
        _staticGeometries;

    public:
        [[deprecated]] void markAsUsed(
            ptr<StaticGeometryResource> resource_,
            mref<StaticGeometrySubResource> subresource_
        );

        [[deprecated]] void unmark(ptr<StaticGeometryResource> resource_, mref<StaticGeometrySubResource> subresource_);

    private:
        /**
         * Materials
         */
        using material_spec_type = __restricted_ptr<const void>;
        using material_map_type = smr<MaterialResource>;
        using material_pass_type = smr<const acc::AccelerationPipeline>;

        // TODO: We need a solution to track the usage markings for spec (1) -> material (2) -> pass (-)
        RobinMap<
            material_spec_type,
            RobinMap<
                __restricted_ptr<MaterialResource>,
                uptr<CacheCtrlSubject<_STD pair<material_map_type, material_pass_type>>>
            >
        > _materialPasses;

    public:
        void markAsUsed(
            _In_ const __restricted_ptr<const void> spec_,
            _In_ mref<smr<MaterialResource>> material_,
            _In_ mref<smr<const acc::AccelerationPipeline>> accelerationPipeline_
        );

        void unmark(
            _In_ const __restricted_ptr<const material_spec_type> spec_,
            _In_ const __restricted_ptr<MaterialResource> material_
        );

        [[nodiscard]] query_result_type<material_pass_type> query(
            _In_ const __restricted_ptr<const void> spec_,
            _In_ const __restricted_ptr<MaterialResource> material_
        ) const noexcept;

        void drop(mref<material_spec_type> spec_);
    };
}
