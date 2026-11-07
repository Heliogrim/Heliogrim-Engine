#include "Layout.hpp"

#include <algorithm>
#include <limits>
#include <string>
#include <tuple>

using namespace hg::engine::accel::lang;
using namespace hg;

namespace {
	constexpr u32 maxDepth = 256uL;
	constexpr size_t maxKey = 4096uz;
	constexpr u64 maxSize = std::numeric_limits<u32>::max();

	[[nodiscard]] constexpr u64 roundUp(u64 value_, u64 align_) {
		return (value_ + align_ - 1uLL) / align_ * align_;
	}

	[[nodiscard]] constexpr u32 clamp32(u64 value_) {
		return static_cast<u32>(std::min(value_, maxSize));
	}

	/* std430 facts of one type; `key` is its structural identity (empty when too long or not laid out) */
	struct Info {
		u64 valid : 1 = 0uLL;
		u64 unsized : 1 = 0uLL;
		u64 opaque : 1 = 0uLL;
		/* at most maxSize */
		u64 locations : 61 = 1uLL;
		u64 size = 0uLL;
		u64 align = 1uLL;
		String key;
	};

	class Types {
	public:
		explicit Types(ref<const Vector<ir::TypeDesc>> types_) :
			_types(types_),
			_memo(types_.size()),
			_state(types_.size(), State::eNew) {}

		[[nodiscard]] ref<const Info> of(ir::TypeRef ref_) {
			return info(ref_.index, 0uL);
		}

		[[nodiscard]] ref<const ir::TypeDesc> desc(ir::TypeRef ref_) const {
			return _types[ref_.index];
		}

	private:
		enum class State : u8 {
			eNew,
			eOpen,
			eDone
		};

		ref<const Vector<ir::TypeDesc>> _types;
		Vector<Info> _memo;
		Vector<State> _state;
		Info _invalid {};

		/* returns a ref into `_memo`, which never reallocates */
		[[nodiscard]] ref<const Info> info(u32 index_, u32 depth_) {
			// an open type is on the current path: a cycle
			if (index_ >= _types.size() || depth_ > maxDepth || _state[index_] == State::eOpen) {
				return _invalid;
			}
			if (_state[index_] == State::eNew) {
				_state[index_] = State::eOpen;
				auto result = compute(_types[index_], depth_);
				if (not result.valid || result.size > maxSize) {
					result = Info {};
				}
				if (result.key.size() > maxKey) {
					result.key.clear();
				}
				_memo[index_] = std::move(result);
				_state[index_] = State::eDone;
			}
			return _memo[index_];
		}

		[[nodiscard]] static Info scalar(ref<const ir::TypeDesc> desc_) {
			Info out {};
			if (desc_.scalar == ir::ScalarKind::eBool) {
				out = Info { .valid = true, .size = 4uLL, .align = 4uLL, .key = "bool" };
			} else if (desc_.bits == 8uL || desc_.bits == 16uL || desc_.bits == 32uL || desc_.bits == 64uL) {
				const auto prefix = desc_.scalar == ir::ScalarKind::eFloat ? "f" : desc_.scalar == ir::ScalarKind::eInt ? "s" : "u";
				out = Info { .valid = true, .size = desc_.bits / 8uLL, .align = desc_.bits / 8uLL, .key = prefix + std::to_string(desc_.bits) };
			}
			return out;
		}

		/* n components of `component_` */
		[[nodiscard]] static Info vector(ref<const Info> component_, u32 n_) {
			if (not component_.valid || n_ < 2uL || n_ > 4uL) {
				return Info {};
			}
			const auto c = component_.size;
			return Info {
				.valid = true,
				.locations = c == 8uLL && n_ >= 3uL ? 2uLL : 1uLL,
				.size = n_ * c,
				.align = (n_ == 3uL ? 4uLL : n_) * c,
				.key = component_.key + "vec" + std::to_string(n_)
			};
		}

		[[nodiscard]] static Info matrix(ref<const ir::TypeDesc> desc_) {
			const auto column = vector(scalar(desc_), desc_.m);
			if (not column.valid || desc_.n < 2uL || desc_.n > 4uL) {
				return Info {};
			}
			// column stride = column alignment (vec3 columns pad to 16)
			return Info {
				.valid = true,
				.locations = desc_.n * column.locations,
				.size = desc_.n * column.align,
				.align = column.align,
				.key = scalar(desc_).key + "mat" + std::to_string(desc_.n) + "x" + std::to_string(desc_.m)
			};
		}

		[[nodiscard]] Info array(ref<const ir::TypeDesc> desc_, u32 depth_) {
			const auto& element = info(desc_.element.index, depth_ + 1uL);
			if (not element.valid || element.unsized || element.opaque) {
				return Info {};
			}
			const auto stride = roundUp(element.size, element.align);
			const auto count = desc_.count == 0uL ? 1uLL : desc_.count;
			return Info {
				.valid = true,
				.unsized = desc_.count == 0uL,
				// element locations are at most maxSize, so the product fits u64
				.locations = std::min(count * element.locations, maxSize),
				.size = desc_.count * stride,
				.align = element.align,
				.key = element.key.empty() ?
					String {} :
					element.key + "[" + (desc_.count == 0uL ? String {} : std::to_string(desc_.count)) + "]"
			};
		}

		[[nodiscard]] Info structure(ref<const ir::TypeDesc> desc_, u32 depth_) {
			Info out { .valid = true, .key = "{" };
			u64 end = 0uLL;
			for (auto i = 0uz; i < desc_.fields.size(); ++i) {
				const auto& field = desc_.fields[i];
				const auto& member = info(field.type.index, depth_ + 1uL);
				// unsized only as the last member and only as a direct runtime array
				const auto badUnsized = member.unsized &&
					(i + 1uz != desc_.fields.size() || _types[field.type.index].kind != ir::TypeKind::eArray);
				if (not member.valid || member.opaque || badUnsized || roundUp(end, member.align) + member.size > maxSize) {
					out.valid = false;
					break;
				}
				end = roundUp(end, member.align) + member.size;
				out.align = std::max(out.align, member.align);
				out.unsized = member.unsized;
				if (member.key.empty() || out.key.size() > maxKey) {
					out.key.clear();
				} else if (not out.key.empty()) {
					out.key += field.name + ":" + member.key + ";";
				}
			}
			if (not out.key.empty()) {
				out.key += "}";
			}
			out.size = roundUp(end, out.align);
			return out;
		}

		[[nodiscard]] Info compute(ref<const ir::TypeDesc> desc_, u32 depth_) {
			using enum ir::TypeKind;
			if (desc_.kind == eScalar) {
				return scalar(desc_);
			}
			if (desc_.kind == eVector) {
				return vector(scalar(desc_), desc_.n);
			}
			if (desc_.kind == eMatrix) {
				return matrix(desc_);
			}
			if (desc_.kind == eArray) {
				return array(desc_, depth_);
			}
			if (desc_.kind == eStruct) {
				return structure(desc_, depth_);
			}
			if (desc_.kind == eTexture) {
				return Info { .valid = true, .opaque = true, .key = "texture" + std::to_string(static_cast<u32>(desc_.dim)) };
			}
			if (desc_.kind == eSampler) {
				return Info { .valid = true, .opaque = true, .key = "sampler" };
			}
			return Info {};
		}
	};

	enum class Category : u8 {
		eSkip,
		eSlot,
		ePush,
		eForward
	};

	[[nodiscard]] Category categoryOf(ref<const ir::InterfaceEntry> entry_) {
		if (entry_.mode == ir::BindMode::eForward) {
			return entry_.input ? Category::eForward : Category::eSkip;
		}
		if (entry_.cls == ir::BindClass::ePushConstant) {
			return Category::ePush;
		}
		const auto resource = entry_.cls >= ir::BindClass::eUniformBuffer && entry_.cls <= ir::BindClass::eSampler;
		return resource ? Category::eSlot : Category::eSkip;
	}

	struct Item {
		const ir::InterfaceEntry* entry;
		const Info* info;
	};

	/* sorts by (id, class, type key, size, locations, valid) and keeps the first per id */
	void collapse(ref<Vector<Item>> items_) {
		// bitfields cannot be tied, so locations and valid are copied
		const auto rank = [](ref<const Item> item_) {
			return std::tuple<ref<const String>, ir::BindClass,ref<const String>, u64, u64, u64>(
				item_.entry->id,
				item_.entry->cls,
				item_.info->key,
				item_.info->size,
				item_.info->locations,
				item_.info->valid
			);
		};
		std::ranges::sort(items_, [&](ref<const Item> a_, ref<const Item> b_) { return rank(a_) < rank(b_); });
		const auto dup = std::ranges::unique(items_, [](ref<const Item> a_, ref<const Item> b_) { return a_.entry->id == b_.entry->id; });
		items_.erase(dup.begin(), dup.end());
	}

	/* next free binding per set, sorted by set */
	class Tops {
		Vector<std::pair<u32, u32>> _tops;

		[[nodiscard]] auto find(u32 set_) {
			return std::ranges::lower_bound(_tops, set_, {}, &std::pair<u32, u32>::first);
		}

	public:
		[[nodiscard]] u32 of(u32 set_) {
			const auto it = find(set_);
			return it != _tops.end() && it->first == set_ ? it->second : 0uL;
		}

		void raise(u32 set_, u32 top_) {
			const auto it = find(set_);
			if (it != _tops.end() && it->first == set_) {
				it->second = std::max(it->second, top_);
			} else {
				_tops.insert(it, { set_, top_ });
			}
		}

		[[nodiscard]] u32 highestSet() const {
			return _tops.empty() ? 0uL : _tops.back().first;
		}
	};
}

Vector<u32> ir::std430Offsets(ref<const Vector<TypeDesc>> types_, TypeRef struct_) {
	Types types { types_ };
	const auto& info = types.of(struct_);
	if (not info.valid || types.desc(struct_).kind != TypeKind::eStruct) {
		return {};
	}
	Vector<u32> offsets {};
	u64 end = 0uLL;
	for (const auto& field : types.desc(struct_).fields) {
		const auto& member = types.of(field.type);
		const auto offset = roundUp(end, member.align);
		offsets.emplace_back(clamp32(offset));
		end = offset + member.size;
	}
	return offsets;
}

Vector<ir::SpecConstant> ir::assignSpecConstants(ref<const Vector<ConstantDecl>> constants_) {
	Vector<String> ids {};
	for (const auto& constant : constants_) {
		if (constant.specId.has_value()) {
			ids.emplace_back(*constant.specId);
		}
	}
	std::ranges::sort(ids);
	ids.erase(std::ranges::unique(ids).begin(), ids.end());

	Vector<SpecConstant> result {};
	for (auto i = 0uz; i < ids.size(); ++i) {
		result.emplace_back(SpecConstant { ids[i], static_cast<u32>(i) });
	}
	return result;
}

u32 ir::locationCount(ref<const Vector<TypeDesc>> types_, TypeRef type_) {
	Types types { types_ };
	const auto& info = types.of(type_);
	return info.valid ? clamp32(std::max(info.locations, 1uLL)) : 1uL;
}

ir::Layout ir::assignLayout(
	ref<const Vector<InterfaceEntry>> entries_,
	ref<const Vector<TypeDesc>> types_,
	ref<const LayoutWeights> weights_,
	ref<const Vector<Layout>> existing_
) {
	Types types { types_ };
	Vector<Item> resources {};
	Vector<Item> forward {};
	for (const auto& entry : entries_) {
		const auto category = categoryOf(entry);
		if (category == Category::eSkip) {
			continue;
		}
		auto& into = category == Category::eForward ? forward : resources;
		into.emplace_back(Item { &entry, &types.of(entry.type) });
	}
	collapse(resources);
	collapse(forward);

	Layout layout {};

	// push ranges, id order
	u64 cursor = 0uLL;
	for (const auto& item : resources) {
		if (item.entry->cls != BindClass::ePushConstant) {
			continue;
		}
		const auto& info = *item.info;
		const auto fixed = info.valid && not info.unsized && not info.opaque;
		const auto offset = roundUp(cursor, std::max(fixed ? info.align : 1uLL, 4uLL));
		const auto size = fixed ? roundUp(info.size, 4uLL) : 0uLL;
		layout.push.emplace_back(PushRange { item.entry->id, clamp32(offset), clamp32(size) });
		cursor = offset + size;
	}

	// slots: class grouping, then id; identity = class and type key
	Vector<std::pair<Item, String>> slots {};
	for (const auto& item : resources) {
		if (item.entry->cls != BindClass::ePushConstant) {
			const auto& key = item.info->key;
			slots.emplace_back(item, key.empty() ? String {} : std::to_string(static_cast<u32>(item.entry->cls)) + ":" + key);
		}
	}
	std::ranges::stable_sort(slots, {}, [](ref<const std::pair<Item, String>> slot_) { return slot_.first.entry->cls; });

	// reuse candidates: every distinct slot of the existing layouts with an identity, sorted
	Vector<const SlotAssignment*> pool {};
	for (const auto& other : existing_) {
		for (const auto& slot : other.slots) {
			// ~0 would overflow the next free binding
			if (not slot.identity.empty() && slot.set != ~0uL && slot.binding != ~0uL) {
				pool.emplace_back(&slot);
			}
		}
	}
	const auto poolKey = [](const SlotAssignment* s_) { return std::tie(s_->set, s_->binding, s_->identity, s_->id); };
	std::ranges::sort(pool, [&](auto a_, auto b_) { return poolKey(a_) < poolKey(b_); });
	const auto dup = std::ranges::unique(pool, [&](auto a_, auto b_) { return poolKey(a_) == poolKey(b_); });
	pool.erase(dup.begin(), dup.end());

	Tops tops {};
	Vector<std::pair<u32, u32>> taken {};
	const auto isTaken = [&](u32 set_, u32 binding_) {
		return std::ranges::binary_search(taken, std::pair { set_, binding_ });
	};
	const auto take = [&](ref<const std::pair<Item, String>> slot_, u32 set_, u32 binding_) {
		taken.insert(std::ranges::lower_bound(taken, std::pair { set_, binding_ }), { set_, binding_ });
		tops.raise(set_, binding_ + 1uL);
		layout.slots.emplace_back(SlotAssignment { slot_.first.entry->id, set_, binding_, slot_.second });
	};

	// 1. reuse, globally fewest skips first, so a reuse is not charged for a hole a later reuse fills
	Vector<bool> reused(slots.size(), false);
	for (;;) {
		const SlotAssignment* best = nullptr;
		std::tuple<u32, bool, u32, u32, size_t> bestRank {};
		for (size_t i = 0uz; i < slots.size(); ++i) {
			const auto& [item, identity] = slots[i];
			if (reused[i] || identity.empty()) {
				continue;
			}
			for (const auto* candidate : pool) {
				if (candidate->identity != identity || isTaken(candidate->set, candidate->binding)) {
					continue;
				}
				const auto top = tops.of(candidate->set);
				const u32 skipped = candidate->binding > top ? candidate->binding - top : 0uL;
				const auto rank = std::tuple<u32, bool, u32, u32, size_t>(
					skipped,
					candidate->id != item.entry->id,
					candidate->set,
					candidate->binding,
					i
				);
				if (best == nullptr || rank < bestRank) {
					best = candidate;
					bestRank = rank;
				}
			}
		}
		if (best == nullptr || not(weights_.unusedSlotCost * static_cast<float>(std::get<0>(bestRank)) < weights_.extraLayoutCost)) {
			break;
		}
		const auto i = std::get<4>(bestRank);
		reused[i] = true;
		take(slots[i], best->set, best->binding);
	}
	Vector<const std::pair<Item, String>*> fresh {};
	for (size_t i = 0uz; i < slots.size(); ++i) {
		if (not reused[i]) {
			fresh.emplace_back(&slots[i]);
		}
	}

	// 2. new bindings; the skip cost is the holes actually left in set 0 (taken entries are unique and below its top)
	const auto takenInSetZero = std::ranges::count(taken, 0uL, &std::pair<u32, u32>::first);
	const auto skipCost = weights_.unusedSlotCost * static_cast<float>(tops.of(0uL) - static_cast<u32>(takenInSetZero));
	u32 set = 0uL;
	if (not fresh.empty() && weights_.extraSetCost < skipCost && tops.highestSet() != ~0uL) {
		set = tops.highestSet() + 1uL;
	}
	for (const auto* slot : fresh) {
		take(*slot, set, tops.of(set));
	}

	std::ranges::sort(layout.slots, {}, [](ref<const SlotAssignment> s_) { return std::pair { s_.set, s_.binding }; });

	// forward input locations, id order
	u64 location = 0uLL;
	for (const auto& item : forward) {
		layout.inputLocations.emplace_back(clamp32(location));
		location += item.info->valid ? std::max(item.info->locations, 1uLL) : 1uLL;
	}
	return layout;
}
