#pragma once

#include <Engine.Common/Move.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Storage.Registry/IStorage.hpp>

#include "../Access/IoResourceBase.hpp"

namespace hg::engine::storage {
	struct ResourceStore {
		struct StoredResource {
			Rc<IoResourceBase> data;
			type_id type;
		};

		DenseMap<Arci<IStorage>, StoredResource> map;

		template <class Type_>
		decltype(auto) add(_In_ mref<Arci<IStorage>> key_, mref<Rc<Type_>> resource_) {
			::hg::assertrt(not map.contains(key_));
			const auto result = map.emplace(
				::hg::move(key_),
				StoredResource { (Rc<IoResourceBase>&&)::hg::move(resource_), reflect::typeId<Type_>() }
			);
			return (const Rc<Type_>&)(result.first->second.data);
		}

		decltype(auto) has(_In_ mref<Arci<IStorage>> key_) const {
			return map.contains(key_);
		}

		template <class Type_>
		decltype(auto) get(_In_ mref<Arci<IStorage>> key_) const {
			::hg::assertrt(map.contains(key_));
			const auto stored = map.at(::hg::move(key_));
			return stored.type == reflect::typeId<Type_>() ? None : Some((Rc<Type_>&&)clone(stored.data));
		}

		decltype(auto) drop(_In_ mref<Arci<IStorage>> key_) {
			::hg::assertrt(map.contains(key_));
			const auto where = map.find(key_);
			auto stored = hg::move(where->second);
			map.erase(where);
			return stored.data;
		}
	};
}
