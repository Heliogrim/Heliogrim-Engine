#pragma once

#include "GlobalCacheCtrl.hpp"
#include "LocalResourceCache.hpp"

namespace ember::engine::gfx::cache {
    template <
        class SubjectType_,
        class SubResourceType_
    >
    struct LocalCacheCtrlSubject {
    public:
        using this_type = LocalCacheCtrlSubject<SubjectType_, SubResourceType_>;
        using subject_type = SubjectType_;
        using sub_resource_type = SubResourceType_;

    public:
        ptr<subject_type> subject;
        sub_resource_type subResource;
    };

    class LocalCacheCtrl final {
    public:
        friend class LocalResourceCache;

    public:
        using this_type = LocalCacheCtrl;

        using texture_subject_type = LocalCacheCtrlSubject<TextureResource, AssocKey<TextureSubResource>>;
        using static_geometry_subject_type = LocalCacheCtrlSubject<StaticGeometryResource, StaticGeometrySubResource>;

    public:
        LocalCacheCtrl(mref<uptr<LocalResourceCache>> cache_);

        LocalCacheCtrl(cref<this_type>) = delete;

        LocalCacheCtrl(mref<this_type>) noexcept = delete;

        ~LocalCacheCtrl();

    public:
        ref<this_type> operator=(cref<this_type>) = delete;

        ref<this_type> operator=(mref<this_type>) noexcept = delete;

    public:
        void tidy();

        void reset(const bool fully_ = false);

    private:
        uptr<LocalResourceCache> _cache;

    public:
        [[nodiscard]] const non_owning_rptr<const LocalResourceCache> cache() const noexcept;

        [[nodiscard]] const non_owning_rptr<LocalResourceCache> cache() noexcept;

    private:
        Vector<texture_subject_type> _textures;
        Vector<static_geometry_subject_type> _staticGeometries;

    public:
        void markAsUsed(const ptr<TextureResource> resource_, mref<TextureSubResource> subResource_);

        void markAsUsed(const ptr<TextureResource> resource_, cref<AssocKey<TextureSubResource>> subResource_);

        void markAsUsed(const ptr<StaticGeometryResource> resource_, mref<StaticGeometrySubResource> subResource_);
    };
}
