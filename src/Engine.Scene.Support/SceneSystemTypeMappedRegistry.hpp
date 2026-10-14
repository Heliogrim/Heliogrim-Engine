#pragma once
#include <ranges>

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Reflect/Inherit/Concept.hpp>
#include <Engine.Reflect/Meta/MetaClass.hpp>

#include "SceneSystemTypeMappedStorage.hpp"

namespace hg::engine::scene {
	namespace {
		struct hash_helper {
			[[nodiscard]] size_t operator()(const ptr<const MetaClass> obj_) const noexcept {
				constexpr auto ih = ankerl::unordered_dense::v4_5_0::hash<std::uintptr_t> {};
				return ih(reinterpret_cast<std::uintptr_t>(obj_));
			}
		};

		template <typename Type_, typename Base_>
		concept ValidMappedStorageType = ClassHasMeta<Type_> &&
			std::derived_from<Type_, Base_> &&
			std::is_move_assignable_v<Type_> &&
			std::is_move_constructible_v<Type_>;

		template <typename Fn_, class Type_>
		concept ValidMappedForEachImm = requires(ref<const Type_> data_) {
			{ std::declval<std::decay_t<Fn_>>()(data_) };
		} && std::is_invocable_v<std::decay_t<Fn_>, ref<const Type_>>;

		template <typename Fn_, class Type_>
		concept ValidMappedPredicate = requires(ref<const Type_> data_) {
			{ std::declval<std::decay_t<Fn_>>()(data_) } -> std::same_as<bool>;
		};
	}

	template <typename BaseModelType_>
	class SceneSystemTypeMappedRegistry {
	public:
		using this_type = SceneSystemTypeMappedRegistry<BaseModelType_>;
		using mapped_storage_type = SceneSystemTypeMappedStorageBase<BaseModelType_>;

		template <typename Derived_>
		using type_mapped_storage_type = SceneSystemTypeMappedStorage<Derived_, BaseModelType_>;

	protected:
		Vector<std::pair<const ptr<const MetaClass>, const ptr<const MetaClass>>> _mappings;
		DenseMap<const ptr<const MetaClass>, uptr<mapped_storage_type>, hash_helper> _storageMapping;

	public:
		template <ClassHasMeta SrcType_, ValidMappedStorageType<BaseModelType_> DstType_> requires
			std::is_constructible_v<DstType_, ref<SrcType_>>
		void addTypeMapping() {

			const auto* const srcMetaClass = SrcType_::getStaticMetaClass();
			const auto* const dstMetaClass = DstType_::getStaticMetaClass();

			_mappings.emplace_back(std::make_pair(srcMetaClass, dstMetaClass));

			/**/

			if (_storageMapping.contains(dstMetaClass)) {
				return;
			}
			_storageMapping.emplace(dstMetaClass, make_uptr<type_mapped_storage_type<DstType_>>());
		}

		[[nodiscard]] decltype(auto) getTypeMappings() const {
			return _mappings;
		}

		[[nodiscard]] decltype(auto) getTypeStorages() {
			return std::views::values(_storageMapping.values());
		}

		[[nodiscard]] Opt<ref<mapped_storage_type>> findTypeStorage(const ptr<const MetaClass> metaClass_) noexcept {
			const auto result = _storageMapping.find(metaClass_);
			return result == _storageMapping.end() ? None : Some<ref<mapped_storage_type>>(*result->second);
		}

	protected:
		template <ValidMappedStorageType<BaseModelType_> Type_>
		[[nodiscard]] const auto* const assertGetStorage() const {

			const auto* const metaClass = Type_::getStaticMetaClass();
			const auto storageIt = _storageMapping.find(metaClass);

			::hg::assertrt(storageIt != _storageMapping.end());

			const auto* const storageBase = storageIt->second.get();
			return static_cast<const ptr<const type_mapped_storage_type<Type_>>>(storageBase);
		}

	public:
		template <ValidMappedStorageType<BaseModelType_> Type_, typename Fn_> requires ValidMappedForEachImm<Fn_, Type_>
		void forEach(Fn_&& fn_) const {
			assertGetStorage<Type_>()->forEach(std::forward<Fn_>(fn_));
		}
	};
}
