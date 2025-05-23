#include "LocalCacheCtrl.hpp"

#include <Engine.Accel.Pipeline/AccelerationPipeline.hpp>

#include "GlobalResourceCache.hpp"
#include "LocalResourceCache.hpp"

using namespace hg::engine::gfx::cache;
using namespace hg;

LocalCacheCtrl::LocalCacheCtrl(mref<uptr<LocalResourceCache>> cache_) noexcept :
	_cache(std::move(cache_)) {}

LocalCacheCtrl::~LocalCacheCtrl() {
	tidy();
}

void LocalCacheCtrl::tidy() {

	if (_cache.get()) {
		reset(true);
		_cache.reset();
	}
}

void LocalCacheCtrl::reset(const bool fully_) {

	auto global { _cache->_global };

	/**
	 * Reset all texture marks
	 */
	for (auto&& entry : _textures) {
		global->unmark(entry.subject, entry.subResource);
	}
	_textures.clear();

	/**
	 * Reset all geometry marks
	 */
	for (auto&& entry : _staticGeometries) {
		global->unmark(entry.subject, std::move(entry.subResource));
	}
	_staticGeometries.clear();

	/**
	 * Propagate reset
	 */
	_cache->reset(fully_);
}

const non_owning_rptr<const LocalResourceCache> LocalCacheCtrl::cache() const noexcept {
	return _cache.get();
}

const non_owning_rptr<LocalResourceCache> LocalCacheCtrl::cache() noexcept {
	return _cache.get();
}

void LocalCacheCtrl::markLoadedAsUsed(mref<smr<TextureResource>> resource_, mref<TextureSubResource> subResource_) {

	auto subKey = AssocKey<TextureSubResource>::from(std::move(subResource_));
	_textures.push_back({ resource_.get(), subKey });

	/**/

	[[maybe_unused]] auto result = _cache->_global->markLoadedAsUsed(std::move(resource_), subKey);
}

void LocalCacheCtrl::markLoadedAsUsed(
	mref<smr<TextureResource>> resource_,
	cref<AssocKey<TextureSubResource>> subResource_
) {

	_textures.push_back({ resource_.get(), subResource_ });

	/**/

	[[maybe_unused]] auto result = _cache->_global->markLoadedAsUsed(std::move(resource_), std::move(subResource_));
}

void LocalCacheCtrl::markAsUsed(const ptr<TextureResource> resource_, mref<TextureSubResource> subResource_) {
	markAsUsed(resource_, AssocKey<TextureSubResource>::from(std::move(subResource_)));
}

void LocalCacheCtrl::markAsUsed(
	const ptr<TextureResource> resource_,
	cref<AssocKey<TextureSubResource>> subResource_
) {
	// TODO: Check for uniqueness
	_textures.push_back({ resource_, subResource_ });
	_cache->_global->markAsUsed(resource_, subResource_);
}

void LocalCacheCtrl::markAsUsed(
	const ptr<StaticGeometryResource> resource_,
	mref<StaticGeometrySubResource> subResource_
) {

	// TODO: Check for uniqueness
	_staticGeometries.push_back({ resource_, subResource_ });
	_cache->_global->markAsUsed(resource_, std::move(subResource_));
}

void LocalCacheCtrl::markAsUsed(
	const non_owning_rptr<const void> spec_,
	mref<smr<MaterialResource>> material_,
	mref<smr<accel::AccelerationPipeline>> accelerationPipeline_
) {
	_specMaterials.push_back({ spec_, { material_.get(), accelerationPipeline_.get() } });

	/**/

	_cache->_global->markAsUsed(
		spec_,
		std::move(material_),
		std::move(accelerationPipeline_)
	);
}
