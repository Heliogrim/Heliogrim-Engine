#pragma once

#include <ranges>
#include <Engine.Common/Move.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Collection/Array.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Storage.Registry/IStorage.hpp>

namespace hg::engine::storage {
	struct ResourceCache {
		struct CachedResource {
			using address_type = uintptr_t;
			using dep_type = std::pair<type_id, address_type>;

			Rc<void> data;
			type_id type;

			// TODO: Maybe replace with InlineAutoArray<T, 2> as we at most expect 2 layers of depth?
			Vector<dep_type> deps;
		};

		DenseMap<Arci<IStorage>, CachedResource> map;

		template <class Type_>
		decltype(auto) add(
			_In_ mref<Arci<IStorage>> key_,
			_In_ mref<Rc<Type_>> resource_,
			_In_ auto*... deps_
		) {
			// Error: Duplicated key...
			::hg::assertrt(not map.contains(key_));
			const auto result = map.emplace(
				::hg::move(key_),
				CachedResource {
					(Rc<void>&&)::hg::move(resource_)/* TODO: .into<void>()*/,
					reflect::typeId<Type_>(),
					{
						CachedResource::dep_type {
							reflect::typeId<std::remove_pointer_t<decltype(deps_)>>(),
							reinterpret_cast<CachedResource::address_type>(deps_)
						}...
					}
				}
			);
			return (const Rc<Type_>&)(result.first->second.data);
		}

		decltype(auto) has(_In_ mref<Arci<IStorage>> key_) const {
			return map.contains(key_);
		}

		bool valid(_In_ mref<Arci<IStorage>> key_, _In_ auto*... deps_) const {

			const auto observed = std::array {
				CachedResource::dep_type {
					reflect::typeId<std::remove_pointer_t<decltype(deps_)>>(),
					reinterpret_cast<CachedResource::address_type>(deps_)
				}...
			};

			const auto where = map.find(::hg::move(key_));
			return where != map.end() && where->second.deps.size() == observed.size() && [](
				const auto& left_,
				const auto& right_
			) {
					for (const auto idx : std::views::iota(sizeof...(deps_))) {
						if (left_[idx] != right_[idx]) {
							return false;
						}
					}
					return true;
				}(observed, where->second.deps);
		}

		template <class Type_>
		decltype(auto) get(
			_In_ mref<Arci<IStorage>> key_,
			_In_ auto*... deps_
		) const {
			::hg::assertrt(map.contains(key_));

			const auto stored = map.at(::hg::move(key_));
			if (stored.type != reflect::typeId<Type_>()) {
				return None;
			}

			const auto set = std::array {
				CachedResource::dep_type {
					reflect::typeId<std::remove_pointer_t<decltype(deps_)>>(),
					reinterpret_cast<CachedResource::address_type>(deps_)
				}...
			};
			if (set.size() != stored.deps.size()) {
				return None;
			}

			for (const auto idx : std::views::iota(sizeof...(deps_))) {
				if (set[idx] != stored.deps[idx]) {
					return None;
				}
			}
			return Some(stored.data);
		}

		decltype(auto) invalidate(_In_ mref<Arci<IStorage>> key_) {
			// Error: We need to maintain the Rc<T> until all references got removed
			::hg::assertrt(map.contains(key_));
			map.erase(::hg::move(key_));
		}
	};
}
