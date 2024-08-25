#pragma once

#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Accel.Storage/GlobalStorage.hpp>
#include <Engine.Resource/Loader/Cache.hpp>

#include "__fwd.hpp"
#include "Traits.hpp"

namespace hg::engine::accel::loader {
	class EffectCache final :
		public resource::loader::Cache<assets::AccelEffect, EffectResource> {
	public:
		using this_type = EffectCache;
		using underlying_type = resource::loader::Cache<assets::AccelEffect, EffectResource>;

		using underlying_type::loader_traits;
		using underlying_type::request_type;
		using underlying_type::response_type;

	public:
		explicit EffectCache(mref<smr<GlobalStorage>> storage_);

		~EffectCache() override;

	private:
		smr<GlobalStorage> _storage;

	public:
		[[nodiscard]] response_type::type operator()(
			mref<request_type::type> request_,
			mref<request_type::options> options_,
			cref<next_type> next_
		) const override;
	};
}
