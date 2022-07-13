#include "GlobalCacheCtrl.hpp"

using namespace ember::engine::gfx::cache;
using namespace ember;

GlobalCacheCtrl::GlobalCacheCtrl(const ptr<GlobalResourceCache> cache_) :
    _cache(cache_) {}

GlobalCacheCtrl::~GlobalCacheCtrl() = default;

const ptr<GlobalResourceCache> GlobalCacheCtrl::cache() const noexcept {
    return _cache;
}

void GlobalCacheCtrl::markAsUsed(ptr<TextureResource> resource_, mref<CacheTextureSubject> subresource_) {

    using SubjectType = CacheCtrlSubject<CacheTextureSubject>;

    /**
     * Check whether resource is already present within controls
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
    const auto existing {
        _STD ranges::lower_bound(ctrls, subresource_,
            [](cref<CacheTextureSubject> entry_, cref<CacheTextureSubject> value_) {
                return _STD ranges::less {}(entry_, value_);
            }, [](const auto* const it_) {
                return it_->subject;
            })
    };

    if (existing != ctrls.end()) {
        /**
         * Atomically increase mark count at ctrl object
         */
        (*existing)->marks.operator++();

    } else {
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
     * Check whether resource is already present within controls
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
        _STD ranges::lower_bound(ctrls, subresource_,
            [](cref<CacheTextureSubject> entry_, cref<CacheTextureSubject> value_) {
                return _STD ranges::less {}(entry_, value_);
            }, [](const auto* const it_) {
                return it_->subject;
            })
    };

    /**
     * exit if not ctrl object is present
     */
    if (existing == ctrls.end()) {
        return;
    }

    /**
     * Atomically decrease mark count at ctrl object
     */
    (*existing)->marks.operator--();
}

void GlobalCacheCtrl::unmark(ptr<TextureResource>, mref<TextureSubResourceRange> subresourceRange_) {
    throw NotImplementedException();
}

void GlobalCacheCtrl::markAsUsed(ptr<StaticGeometryResource> resource_, mref<CacheStaticGeometrySubject> subresource_) {
    throw NotImplementedException();
}

void GlobalCacheCtrl::unmark(ptr<StaticGeometryResource> resource_, mref<CacheStaticGeometrySubject> subresource_) {
    throw NotImplementedException();
}
