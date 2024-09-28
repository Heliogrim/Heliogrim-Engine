#pragma once

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Functional/Function.hpp>
#include <Engine.Reflect/Inherit/Concept.hpp>
#include <Engine.Reflect/Meta/MetaClass.hpp>

#include "RenderSceneSystemSubStorage.hpp"

namespace hg::engine::render {
	namespace {
		struct hash_helper {
			[[nodiscard]] size_t operator()(const ptr<const MetaClass> obj_) const noexcept {
				constexpr static auto ih = ankerl::unordered_dense::v4_4_0::hash<uintptr_t> {};
				return ih(reinterpret_cast<uintptr_t>(obj_));
			}
		};

		template <typename Type_>
		concept ValidSubStorageType = ClassHasMeta<Type_> &&
			std::is_move_assignable_v<Type_> &&
			std::is_move_constructible_v<Type_>;

		template <typename Fn_, class Type_>
		concept ValidForEachImm = requires(cref<Type_> data_) {
			{ std::declval<std::decay_t<Fn_>>()(data_) };
		} && std::is_invocable_v<std::decay_t<Fn_>, cref<Type_>>;
	}

	struct RenderSceneRegistry {
		Vector<std::pair<const ptr<const MetaClass>, const ptr<const MetaClass>>> mapping;

		DenseMap<const ptr<const MetaClass>, uptr<RenderSceneSystemSubStorageBase>, hash_helper> storageMap;

	public:
		template <class SrcType_, ValidSubStorageType DstType_> requires
			std::is_constructible_v<DstType_, ptr<SrcType_>>
		void addTypeMapping() {

			const auto* const srcMeta = SrcType_::getStaticMetaClass();
			const auto* const dstMeta = DstType_::getStaticMetaClass();

			mapping.emplace_back(std::make_pair(srcMeta, dstMeta));

			/**/

			if (storageMap.contains(dstMeta)) {
				return;
			}

			storageMap.insert(std::make_pair(dstMeta, make_uptr<RenderSceneSystemSubStorage<DstType_>>()));
		}

	public:
		template <ValidSubStorageType Type_, typename Fn_> requires ValidForEachImm<Fn_, Type_>
		void forEach(Fn_&& fn_) {

			const auto* const metaClass = Type_::getStaticMetaClass();
			const auto storageIt = storageMap.find(metaClass);

			assert(storageIt != storageMap.end());

			const auto* const storageBase = storageIt->second.get();
			const auto* const storage = static_cast<const ptr<const RenderSceneSystemSubStorage<Type_>>>(storageBase);

			storage->template forEach<Fn_>(std::forward<Fn_>(fn_));
		}
	};
}
