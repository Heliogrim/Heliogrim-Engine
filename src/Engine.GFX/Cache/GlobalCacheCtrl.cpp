#include "GlobalCacheCtrl.hpp"

#include <Engine.Assets/Types/Texture/Texture.hpp>
#include <Engine.Resource/Loader/StreamLoader.hpp>

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

    using stream_loader_type = res::StreamLoader<assets::Texture>;
    using stream_options_type = _STD remove_pointer_t<stream_loader_type::stream_options_type>;

    auto* options = new stream_options_type {
        .layer = subresource_.layer,
        .mip = subresource_.mip,
        .offset = subresource_.offset,
        .extent = subresource_.extent
    };

    //
    resource_->streamLoad(static_cast<const ptr<EmberObject>>(static_cast<const ptr<void>>(options)));

    delete options;
}

void GlobalCacheCtrl::dispatchUnload(const ptr<TextureResource> resource_, cref<TextureSubResource> subresource_) {
    // TODO:

    using stream_loader_type = res::StreamLoader<assets::Texture>;
    using stream_options_type = _STD remove_pointer_t<stream_loader_type::stream_options_type>;

    auto* options = new stream_options_type {
        .layer = subresource_.layer,
        .mip = subresource_.mip,
        .offset = subresource_.offset,
        .extent = subresource_.extent
    };

    //
    resource_->streamUnload(static_cast<const ptr<EmberObject>>(static_cast<const ptr<void>>(options)));

    delete options;
}

void GlobalCacheCtrl::markAsUsed(ptr<TextureResource> resource_, mref<CacheTextureSubject> subresource_) {

    using SubjectType = CacheCtrlSubject<CacheTextureSubject>;

    /**
     * Ensure that resource is already present within controls
     */
    auto resEntry { _textures.find(resource_) };
    if (resEntry == _textures.end()) {
        auto [entry, assigned] = _textures.insert_or_assign(resource_, Vector<ptr<SubjectType>> {});
        assert(assigned);

        resEntry = _STD move(entry);
    }

    /**
     * Check for existing derived subresource
     */
    auto& ctrls { const_cast<Vector<ptr<SubjectType>>&>(resEntry->second) };
    /*
    const auto existing {
        _STD ranges::lower_bound(ctrls, subresource_,
            [](cref<CacheTextureSubject> entry_, cref<CacheTextureSubject> value_) {
                return _STD ranges::less {}(entry_, value_);
            }, [](const auto* const it_) {
                return it_->subject;
            })
    };
     */
    const auto existing {
        _STD ranges::find(
            ctrls,
            subresource_,
            [](const auto* const it_) {
                return it_->subject;
            }
        )
    };

    if (existing != ctrls.end()) {
        /**
         * Atomically increase mark count at ctrl object
         */
        (*existing)->marks.operator++();

    } else {
        /**
         * Dispatch loading of texture (stream loading if possible/available)
         */
        dispatchLoad(resource_, subresource_);

        /**
         * Create new ctrl object by given subject with intial mark count of 1ui16
         */
        ctrls.push_back(new SubjectType { _STD move(subresource_), 1ui16 });

        /**
         * Resort ctrls with new subscribed subject
         */
        _STD ranges::sort(ctrls, [](const SubjectType* const left_, const SubjectType* const right_) {
            return _STD ranges::less {}(left_->subject, right_->subject);
        });
    }
}

void GlobalCacheCtrl::markAsUsed(ptr<TextureResource>, mref<TextureSubResourceRange> subresourceRange_) {
    throw NotImplementedException();
}

void GlobalCacheCtrl::unmark(ptr<TextureResource> resource_, mref<CacheTextureSubject> subresource_) {

    using SubjectType = CacheCtrlSubject<CacheTextureSubject>;

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
    auto& ctrls { const_cast<Vector<ptr<SubjectType>>&>(resEntry->second) };
    const auto existing {
        _STD ranges::find(ctrls, subresource_, [](const auto* const it_) {
            return it_->subject;
        })
    };

    /**
     * Early exit if ctrl object is not present
     */
    if (existing == ctrls.end()) {
        return;
    }

    /**
     * Atomically decrease mark count at ctrl object
     */
    const auto left { (*existing)->marks.operator--() };

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

        const auto* subject { *existing };
        delete subject;

        ctrls.erase(existing);
    }
}

void GlobalCacheCtrl::unmark(ptr<TextureResource>, mref<TextureSubResourceRange> subresourceRange_) {
    throw NotImplementedException();
}

#pragma endregion

#pragma region StaticGeomtryResource

void GlobalCacheCtrl::dispatchLoad(const ptr<StaticGeometryResource> resource_,
    cref<StaticGeometrySubResource> subresource_) {}

void GlobalCacheCtrl::dispatchUnload(const ptr<StaticGeometryResource> resource_,
    cref<StaticGeometrySubResource> subresource_) {}

void GlobalCacheCtrl::markAsUsed(ptr<StaticGeometryResource> resource_, mref<CacheStaticGeometrySubject> subresource_) {
    throw NotImplementedException();
}

void GlobalCacheCtrl::unmark(ptr<StaticGeometryResource> resource_, mref<CacheStaticGeometrySubject> subresource_) {
    throw NotImplementedException();
}

#pragma endregion
