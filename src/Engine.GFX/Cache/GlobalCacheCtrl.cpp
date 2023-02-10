#include "GlobalCacheCtrl.hpp"

#include <Engine.Assets/Types/Texture/Texture.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Resource/LoaderManager.hpp>
#include <Engine.GFX.Loader/Texture/Traits.hpp>
#include <Engine.GFX.Loader/Texture/TextureLoader.hpp>

#include "GlobalResourceCache.hpp"

using namespace ember::engine::gfx::cache;
using namespace ember;

GlobalCacheCtrl::GlobalCacheCtrl(mref<uptr<GlobalResourceCache>> cache_) :
    _cache(_STD move(cache_)),
    _loader(Engine::getEngine()->getResources()->loader(traits::nothrow)) {}

GlobalCacheCtrl::~GlobalCacheCtrl() = default;

const non_owning_rptr<GlobalResourceCache> GlobalCacheCtrl::cache() const noexcept {
    return _cache.get();
}

const non_owning_rptr<const engine::resource::LoaderManager> GlobalCacheCtrl::loader() const noexcept {
    return _loader;
}

GlobalCacheCtrl::stream_result_type<> GlobalCacheCtrl::markLoadedAsUsed(
    mref<smr<TextureResource>> resource_,
    mref<TextureSubResource> subresource_
) {
    return markLoadedAsUsed(_STD move(resource_), AssocKey<TextureSubResource>::from(_STD move(subresource_)));
}

GlobalCacheCtrl::stream_result_type<> GlobalCacheCtrl::markLoadedAsUsed(
    mref<smr<TextureResource>> resource_,
    cref<AssocKey<TextureSubResource>> subresource_
) {
    const auto markResult = markAsUsed(resource_.get(), subresource_);
    if (markResult == StreamCacheResultType::eResidential) {
        return markResult;
    }

    /* If markResult != eResidential (eTransient || eUndefined) ~ Cache Miss */

    using stream_loader = loader::TextureLoader;
    using stream_options = stream_loader::traits::stream_request::options;

    stream_options options {
        .op = loader::TextureStreamOp::eLoad,
        .layer = subresource_.value.layer,
        .mip = subresource_.value.mip,
        .offset = math::ivec3(subresource_.value.offset),
        .extent = subresource_.value.extent
    };

    _loader->streamImmediately<assets::Texture, TextureResource>(
        smr<TextureResource> { resource_ },
        _STD move(options)
    );

    /**/

    // TODO: Check whether loader chain will subsequentially insert a marker to the subresource set
    // TODO:    or whether we should do this within the loaded markup invocation

    {
        using SubjectType = CacheCtrlSubject<TextureSubResource>;
        using SubMapType = RobinMap<AssocKey<TextureSubResource>, ptr<SubjectType>>;

        const auto resEntry { _textures.find(resource_.get()) };
        auto& ctrls { const_cast<SubMapType&>(resEntry->second) };

        /**
         * Create new ctrl object by given subject with intial mark count of 1ui16
         */
        ctrls.insert_or_assign(subresource_, new SubjectType { _STD move(subresource_), 1ui16 });
    }

    return { StreamCacheResultType::eTransient /* eResidential */ };
}

GlobalCacheCtrl::stream_result_type<> GlobalCacheCtrl::markLoadedAsUsed(
    mref<smr<TextureResource>> resource_,
    mref<TextureSubResourceRange> range_
) {
    throw NotImplementedException();
}

GlobalCacheCtrl::stream_result_type<> GlobalCacheCtrl::markAsUsed(
    const non_owning_rptr<TextureResource> resource_,
    mref<TextureSubResource> subresource_
) {
    return markAsUsed(resource_, AssocKey<TextureSubResource>::from(_STD move(subresource_)));
}

GlobalCacheCtrl::stream_result_type<> GlobalCacheCtrl::markAsUsed(
    const non_owning_rptr<TextureResource> resource_,
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

        return { StreamCacheResultType::eResidential };
    }

    return { StreamCacheResultType::eUndefined };
}

GlobalCacheCtrl::stream_result_type<> GlobalCacheCtrl::markAsUsed(
    ptr<TextureResource>,
    mref<TextureSubResourceRange> subresourceRange_
) {
    throw NotImplementedException();
}

void GlobalCacheCtrl::unmark(mref<smr<TextureResource>> resource_, mref<TextureSubResource> subresource_) {
    unmark(_STD move(resource_), AssocKey<TextureSubResource>::from(_STD move(subresource_)));
}

void GlobalCacheCtrl::unmark(mref<smr<TextureResource>> resource_, cref<AssocKey<TextureSubResource>> subresource_) {

    using SubjectType = CacheCtrlSubject<CacheTextureSubject>;
    using SubMapType = RobinMap<AssocKey<CacheTextureSubject>, ptr<SubjectType>>;

    /**
     * Check whether resource is present within controls
     */
    auto resEntry { _textures.find(resource_.get()) };
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
        using stream_loader = loader::TextureLoader;
        using stream_options = stream_loader::traits::stream_request::options;

        stream_options options {
            .op = loader::TextureStreamOp::eUnload,
            .layer = subresource_.value.layer,
            .mip = subresource_.value.mip,
            .offset = math::ivec3(subresource_.value.offset),
            .extent = subresource_.value.extent
        };

        _loader->streamImmediately<assets::Texture, TextureResource>(
            _STD move(resource_),
            _STD move(options)
        );
    }

    // TODO: Check whether we want to erase subjects with no left marks
    if (left <= 0ui16) {

        const auto* subject { existing->second };
        delete subject;

        ctrls.erase(existing);
    }
}

void GlobalCacheCtrl::unmark(mref<smr<TextureResource>> resource_, mref<TextureSubResourceRange> subresourceRange_) {
    throw NotImplementedException();
}

void GlobalCacheCtrl::markAsUsed(ptr<StaticGeometryResource> resource_, mref<CacheStaticGeometrySubject> subresource_) {
    throw NotImplementedException();
}

void GlobalCacheCtrl::unmark(ptr<StaticGeometryResource> resource_, mref<CacheStaticGeometrySubject> subresource_) {
    throw NotImplementedException();
}
