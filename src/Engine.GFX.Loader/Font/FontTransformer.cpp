#include "FontTransformer.hpp"

#include <Engine.GFX/Pool/GlobalResourcePool.hpp>

#include "Transformer/FreeType.hpp"

using namespace hg::engine::gfx::loader;
using namespace hg;

FontTransformer::FontTransformer(ref<pool::GlobalResourcePool> pool_) :
	_pool(std::addressof(pool_)) {
	transformer::prepareFreeType();
}

FontTransformer::FontTransformer(mref<this_type> other_) noexcept :
	_pool(std::move(other_._pool)) {
	transformer::prepareFreeType();
}

FontTransformer::FontTransformer(cref<this_type> other_) noexcept :
	_pool(other_._pool) {
	transformer::prepareFreeType();
}

FontTransformer::~FontTransformer() {
	transformer::cleanupFreeType();
}

FontTransformer::response_type::type FontTransformer::operator()(
	mref<request_type::type> request_,
	mref<request_type::options> options_,
	cref<next_type> next_
) const {

	auto* asset = static_cast<non_owning_rptr<const assets::Asset>>(request_);
	auto src = next_(std::move(asset), next_type::next_request_type::options {});

	/**/

	using derived_type = resource::UniqueResource<FontResource::value_type>;
	auto dst = make_smr<derived_type>(std::in_place).into<FontResource>();
	auto guard = dst->acquire(resource::ResourceUsageFlag::eAll);

	/**/

	transformer::convertFreeType(
		request_,
		std::move(src),
		guard.mut(),
		_pool->device(),
		options_
	);

	/**/

	return dst;
}
