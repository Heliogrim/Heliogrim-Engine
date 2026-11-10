#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Common/Meta/TypeId.hpp>
#include <Engine.Reflect.Utils/With/WithType.hpp>
#include <Engine.Storage.Registry/IStorage.hpp>

#include "../Access/IoResourceAccessorBase.hpp"

namespace hg::engine::storage {
	struct StageKey {
		ptr<const IStorage> storage;
		type_id resource;

	public:
		[[nodiscard]] bool operator==(ref<const StageKey> other_) const noexcept {
			return storage == other_.storage && resource == other_.resource;
		}
	};

	struct ResolvedStage :
		public ArcFromThis<ResolvedStage> {
	public:
		StageKey key;
		Arci<IStorage> storage;
		meta::WithType<Rc<IoResourceAccessorBase>> access;
		Arci<ResolvedStage> below;
		/* Bottom storage of the chain; copied from `below`, or the stage's own storage. */
		ptr<const IStorage> primary;
	};
}

template <>
struct ankerl::unordered_dense::hash<hg::engine::storage::StageKey> {
	using is_avalanching = void;

	[[nodiscard]] auto operator()(const hg::engine::storage::StageKey& key_) const noexcept -> hg::u64 {
		return detail::hash_impl::mix(
			static_cast<hg::u64>(reinterpret_cast<std::uintptr_t>(key_.storage)),
			static_cast<hg::u64>(key_.resource.data)
		);
	}
};
