#include "GlobalCacheCtrl.hpp"

#include <Engine.Assets/Types/Texture/Texture.hpp>
#include <Engine.GFX.Loader/Texture/Traits.hpp>
#include <Engine.GFX.Loader/Texture/TextureLoader.hpp>

#include "GlobalResourceCache.hpp"

using namespace ember::engine::gfx::cache;
using namespace ember;

GlobalCacheCtrl::GlobalCacheCtrl(const ptr<GlobalResourceCache> cache_) :
    _cache(cache_) {}

GlobalCacheCtrl::~GlobalCacheCtrl() = default;

const ptr<GlobalResourceCache> GlobalCacheCtrl::cache() const noexcept {
    return _cache;
}

#pragma region TextureResource

void GlobalCacheCtrl::dispatchLoad(const ptr<TextureResource> resource_, cref<TextureSubResource> subresource_) {
    // TODO:

    using stream_loader = loader::TextureLoader;
    using stream_options = stream_loader::traits::request::stream_type;

    stream_options options {
        .layer = subresource_.layer,
        .mip = subresource_.mip,
        .offset = math::ivec3(subresource_.offset),
        .extent = subresource_.extent
    };

    _cache->request(..., _STD move(options), ...);
    // resource_->streamLoad(static_cast<const ptr<EmberObject>>(static_cast<const ptr<void>>(options)));
}

void GlobalCacheCtrl::dispatchUnload(const ptr<TextureResource> resource_, cref<TextureSubResource> subresource_) {
    // TODO:

    using stream_loader = loader::TextureLoader;
    using stream_options = stream_loader::traits::request::stream_type;

    stream_options options {
        .layer = subresource_.layer,
        .mip = subresource_.mip,
        .offset = math::ivec3(subresource_.offset),
        .extent = subresource_.extent
    };

    _cache->unload(..., _STD move(options), ...);
    // resource_->streamUnload(static_cast<const ptr<EmberObject>>(static_cast<const ptr<void>>(options)));
}

void GlobalCacheCtrl::markAsUsed(
    ptr<TextureResource> resource_,
    mref<TextureSubResource> subresource_
) {
    markAsUsed(resource_, AssocKey<TextureSubResource>::from(_STD move(subresource_)));
}

void GlobalCacheCtrl::markAsUsed(
    ptr<TextureResource> resource_,
    cref<AssocKey<TextureSubResource>> subresource_
) {

    using SubjectType = CacheCtrlSubject<TextureSubResource>;
    using SubMapType = RobinMap<AssocKey<TextureSubResource>, ptr<SubjectType>>;

    /**
     * Ensure that resource is already present within controls
     */
    auto resEntry { _textures.find(resource_) };
    if (resEntry == _textures.end()) {
        auto [entry, assigned] = _textures.insert_or_assign(resource_, SubMapType {});
        assert(assigned);

        resEntry = _STD move(entry);
    }

    /**
     * Check for existing derived subresource
     */
    auto& ctrls { const_cast<SubMapType&>(resEntry->second) };
    const auto existing { ctrls.find(subresource_) };

    if (existing != ctrls.end()) {
        /**
         * Atomically increase mark count at ctrl object
         */
        (existing->second)->marks.operator++();

    } else {
        /**
         * Dispatch loading of texture (stream loading if possible/available)
         */
        dispatchLoad(resource_, subresource_);

        /**
         * Create new ctrl object by given subject with intial mark count of 1ui16
         */
        ctrls.insert_or_assign(subresource_, new SubjectType { _STD move(subresource_), 1ui16 });
    }
}

void GlobalCacheCtrl::markAsUsed(ptr<TextureResource>, mref<TextureSubResourceRange> subresourceRange_) {
    throw NotImplementedException();
}

void GlobalCacheCtrl::unmark(ptr<TextureResource> resource_, mref<TextureSubResource> subresource_) {
    unmark(resource_, AssocKey<TextureSubResource>::from(_STD move(subresource_)));
}

void GlobalCacheCtrl::unmark(ptr<TextureResource> resource_, cref<AssocKey<TextureSubResource>> subresource_) {

    using SubjectType = CacheCtrlSubject<CacheTextureSubject>;
    using SubMapType = RobinMap<AssocKey<CacheTextureSubject>, ptr<SubjectType>>;

    /**
     * Check whether resource is present within controls
     */
    auto resEntry { _textures.find(resource_) };
    if (resEntry == _textures.end()) {
        return;
    }

    /**
     * Check for existing derived subresource
     */
    auto& ctrls { const_cast<SubMapType&>(resEntry->second) };
    const auto existing { ctrls.find(subresource_) };

    /**
     * Early exit if ctrl object is not present
     */
    if (existing == ctrls.end()) {
        return;
    }

    /**
     * Atomically decrease mark count at ctrl object
     */
    const auto left { (existing->second)->marks.operator--() };

    #if _DEBUG
    if (left > 128ui16) {
        __debugbreak();
    }
    #endif

    /**
     * Dispatch unloading of texture (stream unloading if possible/available)
     */
    if (left <= 0ui16) {
        dispatchUnload(resource_, subresource_);
    }

    // TODO: Check whether we want to erase subjects with no left marks
    if (left <= 0ui16) {

        const auto* subject { existing->second };
        delete subject;

        ctrls.erase(existing);
    }
}

void GlobalCacheCtrl::unmark(ptr<TextureResource>, mref<TextureSubResourceRange> subresourceRange_) {
    throw NotImplementedException();
}

#pragma endregion

#pragma region StaticGeometryResource

void GlobalCacheCtrl::dispatchLoad(
    const ptr<StaticGeometryResource> resource_,
    cref<StaticGeometrySubResource> subresource_
) {}

void GlobalCacheCtrl::dispatchUnload(
    const ptr<StaticGeometryResource> resource_,
    cref<StaticGeometrySubResource> subresource_
) {}

void GlobalCacheCtrl::markAsUsed(ptr<StaticGeometryResource> resource_, mref<CacheStaticGeometrySubject> subresource_) {
    throw NotImplementedException();
}

void GlobalCacheCtrl::unmark(ptr<StaticGeometryResource> resource_, mref<CacheStaticGeometrySubject> subresource_) {
    throw NotImplementedException();
}

#pragma endregion
