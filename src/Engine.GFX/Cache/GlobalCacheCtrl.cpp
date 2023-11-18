#include "GlobalCacheCtrl.hpp"

#include <Engine.Logging/Logger.hpp>
#include <Engine.Assets/Types/Texture/TextureAsset.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Resource/ResourceManager.hpp>
#include <Engine.Resource/LoaderManager.hpp>
#include <Engine.GFX.Loader/Texture/Traits.hpp>
#include <Engine.GFX.Loader/Texture/TextureLoader.hpp>
#include <Engine.GFX.Loader/Material/MaterialLoader.hpp>
#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/GuidFormat.hpp>
#include <Engine.Common/Exception/NotImplementedException.hpp>

#include "GlobalResourceCache.hpp"

using namespace hg::engine::gfx::cache;
using namespace hg;

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
        .offset = subresource_.value.offset,
        .extent = subresource_.value.extent
    };

    _loader->streamImmediately<assets::TextureAsset, TextureResource>(
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

void GlobalCacheCtrl::unmark(
    const non_owning_rptr<TextureResource> resource_,
    cref<AssocKey<TextureSubResource>> subresource_
) {

    #ifdef _DEBUG

    auto texture = static_cast<non_owning_rptr<const assets::TextureAsset>>(resource_->getAssociation());
    if (texture == nullptr) {
        Logger::error("Tried to unmark weak reference to texture resource without associative texture object.");
        return;
    }

    auto result = _cache->query(texture->get_guid());
    if (result != QueryResultType::eHit) {
        Logger::error(
            "Tried to unmark a already uncached texture resource of texture object `{}`",
            texture->getAssetName()
        );
        return;
    }

    unmark(smr<resource::ResourceBase> { result.value<>() }.into<TextureResource>(), subresource_);

    #else

    auto texture = static_cast<non_owning_rptr<const assets::Texture>>(resource_->getAssociation());
    auto result = _cache->query(texture->get_guid());

    unmark(smr<resource::ResourceBase> { result.value<>() }.into<TextureResource>(), subresource_);

    #endif
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
            .offset = subresource_.value.offset,
            .extent = subresource_.value.extent
        };

        _loader->streamImmediately<assets::TextureAsset, TextureResource>(
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

#pragma region Materials

void GlobalCacheCtrl::markAsUsed(
    const __restricted_ptr<const void> spec_,
    mref<smr<MaterialResource>> material_,
    mref<smr<const acc::AccelerationPipeline>> accelerationPipeline_
) {

    using SubjectType = CacheCtrlSubject<_STD pair<smr<MaterialResource>, smr<const acc::AccelerationPipeline>>>;
    using SubMapType = RobinMap<__restricted_ptr<MaterialResource>, uptr<SubjectType>>;

    if (not material_->isLoaded()) {
        IM_CORE_ERROR("Tried to cache mark material resource which is still unloaded.");
        return;
    }

    const auto guard = material_->acquire();
    const auto guid = guard->getGuid();

    const auto result = _cache->store(static_cast<cref<asset_guid>>(guid), clone(material_));

    #if _DEBUG
    if (result == StoreResultType::eFail) {
        IM_CORE_WARNF("Material resource `{}` is already stored to global cache.", encodeGuid4228(guid));
    }
    #endif

    /**/

    auto specIt = _materialPasses.find(spec_);
    if (specIt == _materialPasses.end()) {

        const auto insertResult = _materialPasses.insert(_STD make_pair(spec_, SubMapType {}));
        specIt = insertResult.first;

        assert(insertResult.second);
    }

    /**/

    auto& subMap = const_cast<SubMapType&>(specIt->second);
    auto subIt = subMap.find(material_.get());

    if (subIt == subMap.end()) {

        auto key = material_.get();
        auto val = make_uptr<SubjectType>(
            _STD make_pair(_STD move(material_), _STD move(accelerationPipeline_)),
            1ui16
        );

        subMap.insert(
            _STD make_pair(
                _STD move(key),
                _STD move(val)
            )
        );
        return;
    }

    /**/

    subIt->second->marks.operator++();
}

void GlobalCacheCtrl::unmark(
    const __restricted_ptr<const material_spec_type> spec_,
    const __restricted_ptr<MaterialResource> resource_
) {

    using SubjectType = CacheCtrlSubject<_STD pair<smr<MaterialResource>, smr<const acc::AccelerationPipeline>>>;
    using SubMapType = RobinMap<__restricted_ptr<MaterialResource>, uptr<SubjectType>>;

    if (not resource_->isLoaded()) {
        IM_CORE_ERROR("Tried to unmark material resource with is not loaded.");
        return;
    }

    const auto specIt = _materialPasses.find(spec_);
    if (specIt == _materialPasses.end()) {
        return;
    }

    auto& subMap = const_cast<SubMapType&>(specIt->second);
    const auto subIt = subMap.find(resource_);

    if (subIt == subMap.end()) {
        return;
    }

    const auto left { (subIt->second)->marks.operator--() };

    #if _DEBUG
    // No constexpr, just debug expression
    static const u16 wrapTest { 0xFFFEui16 };
    if (left >= wrapTest) {
        __debugbreak();
    }
    #endif

    if (left > 0ui16) {
        return;
    }

    subMap.erase(subIt);
}

GlobalCacheCtrl::query_result_type<smr<const engine::gfx::acc::AccelerationPipeline>> GlobalCacheCtrl::query(
    const __restricted_ptr<const void> spec_,
    const __restricted_ptr<MaterialResource> material_
) const noexcept {

    const auto specIt = _materialPasses.find(spec_);
    if (specIt == _materialPasses.end()) {
        return { QueryResultType::eMiss };
    }

    const auto& subMap = specIt->second;
    const auto subIt = subMap.find(material_);

    if (subIt == subMap.end()) {
        return { QueryResultType::eMiss };
    }

    return { QueryResultType::eHit, subIt->second->subject.second };
}

void GlobalCacheCtrl::drop(mref<material_spec_type> spec_) {
    // Warning: This could break, due to concurrent access
    _materialPasses.erase(_STD move(spec_));
}

#pragma endregion
