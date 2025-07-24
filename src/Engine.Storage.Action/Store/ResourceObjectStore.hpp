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
	struct ResourceObjectStore {
		struct CachedResource {
			using address_type = uintptr_t;
			using dep_type = std::pair<type_id, address_type>;

			Rc<void> data;
			type_id type;

			// TODO: Maybe replace with InlineAutoArray<T, 2> as we at most expect 2 layers of depth?
			Vector<dep_type> deps;
		};

		DenseMap<Arci<IStorage>, Vector<CachedResource>> map;

		template <class Type_>
		decltype(auto) add(
			_In_ mref<Arci<IStorage>> key_,
			_In_ mref<Rc<Type_>> resource_,
			_In_ auto*... deps_
		) {
			const auto iter = map.find(key_);
			if (iter == map.end()) {
				const auto result = map.emplace(
					::hg::move(key_),
					Vector<CachedResource> {
						CachedResource {
							(Rc<void>&&)::hg::move(resource_)/* TODO: .into<void>()*/,
							::hg::refl::TypeId<Type_>(),
							{
								CachedResource::dep_type {
									::hg::refl::TypeId<std::remove_pointer_t<decltype(deps_)>>(),
									reinterpret_cast<CachedResource::address_type>(deps_)
								}...
							}
						}
					}
				);
				return (const Rc<Type_>&)(result.first->second.front().data);
			}

			/**/

			constexpr auto nextTypeId = ::hg::refl::TypeId<Type_>();

			const auto nextDeps = std::array<CachedResource::dep_type, sizeof...(deps_)> {
				CachedResource::dep_type {
					::hg::refl::TypeId<std::remove_pointer_t<decltype(deps_)>>(),
					reinterpret_cast<CachedResource::address_type>(deps_)
				}...
			};

			for (const auto& res : iter->second) {
				if (res.type != nextTypeId || res.deps.size() != nextDeps.size()) {
					continue;
				}

				bool sameDeps = true;
				for (const auto idx : std::views::iota(size_t {}, sizeof...(deps_))) {
					sameDeps = sameDeps && nextDeps[idx] == res.deps[idx];
				}

				// Problem: Duplicated key identification. We need at least a type diff or dependency diff.
				::hg::assertrt(not sameDeps);
			}

			/**/

			iter->second.emplace_back(
				CachedResource {
					(Rc<void>&&)::hg::move(resource_)/* TODO: .into<void>()*/,
					::hg::refl::TypeId<Type_>(),
					{ nextDeps.begin(), nextDeps.end() }
				}
			);

			return (const Rc<Type_>&)(iter->second.back().data);
		}

		decltype(auto) has(_In_ mref<Arci<IStorage>> key_) const {
			return map.contains(key_);
		}

		bool valid(_In_ mref<Arci<IStorage>> key_, _In_ auto*... deps_) const {

			const auto observed = std::array {
				CachedResource::dep_type {
					::hg::refl::TypeId<std::remove_pointer_t<decltype(deps_)>>(),
					reinterpret_cast<CachedResource::address_type>(deps_)
				}...
			};

			const auto where = map.find(::hg::move(key_));
			return where != map.end() && std::ranges::find_if(
				where->second,
				[&observed](const CachedResource& res_) {
					if (res_.deps.size() != observed.size()) {
						return false;
					}

					for (const auto idx : std::views::iota(size_t {}, sizeof...(deps_))) {
						if (res_.deps[idx] != observed[idx]) {
							return false;
						}
					}

					return true;
				}
			) != std::ranges::end(where->second);
		}

		template <class Type_>
		decltype(auto) get(
			_In_ mref<Arci<IStorage>> key_,
			_In_ auto*... deps_
		) const {
			::hg::assertrt(map.contains(key_));

			const auto& stored = map.at(::hg::move(key_));
			const auto set = std::array {
				CachedResource::dep_type {
					::hg::refl::TypeId<std::remove_pointer_t<decltype(deps_)>>(),
					reinterpret_cast<CachedResource::address_type>(deps_)
				}...
			};

			/**/

			for (auto& res : stored) {

				if ([](const auto& set_, const auto& res_) {
					if (res_.type != ::hg::refl::TypeId<Type_>()) {
						return false;
					}

					if (set_.size() != res_.deps.size()) {
						return false;
					}

					for (const auto idx : std::views::iota(size_t {}, sizeof...(deps_))) {
						if (set_[idx] != res_.deps[idx]) {
							return false;
						}
					}

					return true;
				}(set, res)) {
					return Some(res.data);
				}
			}

			return None;
		}

		decltype(auto) invalidate(_In_ mref<Arci<IStorage>> key_) {
			// Error: We need to maintain the Rc<T> until all references got removed
			//::hg::assertrt(map.contains(key_));
			map.erase(::hg::move(key_));
		}
	};
}
