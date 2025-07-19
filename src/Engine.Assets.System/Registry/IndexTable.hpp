#pragma once

#include <algorithm>
#include <map>
#include <ranges>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/DenseMap.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "Index.hpp"
#include "IndexTrait.hpp"
#include "../__fwd.hpp"

namespace hg::engine::assets::system {
	template <typename>
	class IndexTable;
}

namespace hg::engine::assets::system {
	class IndexTableBase {
	public:
		template <typename>
		friend class ::hg::engine::assets::system::IndexTable;

	public:
		using this_type = IndexTableBase;

	private:
		IndexTableBase(mref<string> uniqueName_) :
			_uniqueName(std::move(uniqueName_)) {}

	public:
		IndexTableBase(mref<this_type>) noexcept = delete;

		IndexTableBase(cref<this_type>) = delete;

		virtual ~IndexTableBase() = default;

	private:
		string _uniqueName;

	public:
		[[nodiscard]] cref<string> getUniqueName() const noexcept {
			return _uniqueName;
		}

	public:
		[[nodiscard]] constexpr virtual bool isUniqueIndex() const noexcept = 0;

		[[nodiscard]] virtual bool slowContains(nmpt<Asset> asset_) const noexcept = 0;

		virtual void clear() = 0;

		virtual void store(nmpt<Asset> asset_) = 0;

		virtual void remove(nmpt<Asset> asset_) = 0;
	};

	template <typename Index_>
	class IndexTable :
		public IndexTableBase {
	public:
		using this_type = IndexTable<Index_>;

		using index_type = Index_;
		using trait_type = IndexTrait<index_type>;

		using allocator_type = std::allocator<typename trait_type::data_type>;
		using allocator_traits = std::allocator_traits<allocator_type>;

	protected:
		IndexTable(mref<string> uniqueName_) :
			IndexTableBase(std::move(uniqueName_)) {}

	public:
		~IndexTable() override = default;
	};

	/**/

	namespace {
		template <typename TypeTraits_, bool TestUnique_, bool TestMultiple_>
		struct AutoMappingSelector;

		template <typename TypeTraits_>
		struct AutoMappingSelector<TypeTraits_, true, false> {
			using table_type = DenseMap<
				typename TypeTraits_::data_type,
				nmpt<Asset>,
				typename TypeTraits_::hash_type
			>;
		};

		template <typename TypeTraits_>
		struct AutoMappingSelector<TypeTraits_, true, true> {
			using table_type = std::map<
				typename TypeTraits_::data_type,
				nmpt<Asset>,
				typename TypeTraits_::relation_type
			>;
		};

		template <typename TypeTraits_>
		struct AutoMappingSelector<TypeTraits_, false, true> {
			using table_type = std::map<
				typename TypeTraits_::data_type,
				Vector<nmpt<Asset>>,
				typename TypeTraits_::relation_type
			>;
		};

		template <typename TypeTraits_>
		struct AutoMappingSelector<TypeTraits_, false, false> {
			static_assert(TypeTraits_::unique || TypeTraits_::multiple);
		};
	};

	/**/

	template <typename Index_>
	class AutoIndexTable :
		public IndexTable<Index_> {
	public:
		using this_type = AutoIndexTable<Index_>;

		using underlying_type = IndexTable<Index_>;

		using index_type = typename underlying_type::index_type;
		using trait_type = typename underlying_type::trait_type;

		using allocator_type = typename underlying_type::allocator_type;
		using allocator_traits = typename underlying_type::allocator_traits;

		/**/

		using auto_type = AutoMappingSelector<trait_type, trait_type::unique, trait_type::multiple>;
		using auto_table_type = typename auto_type::table_type;

	public:
		AutoIndexTable(mref<string> uniqueName_) :
			IndexTable<Index_>(std::move(uniqueName_)),
			_table() {
			static_assert(!trait_type::dynamic);
		}

		~AutoIndexTable() override = default;

	private:
		auto_table_type _table;

	public:
		[[nodiscard]] auto tableKeys() const {
			return std::ranges::views::keys(_table);
		}

		[[nodiscard]] auto tableValues() const {
			return std::ranges::views::values(_table);
		}

	public:
		[[nodiscard]] constexpr bool isUniqueIndex() const noexcept override {
			return trait_type::unique;
		}

		void clear() override {
			_table.clear();
		}

		/**
		 * Query Operations
		 */
	public:
		template <typename IndexType_ = Index_> requires IndexTrait<IndexType_>::unique &&
			(not IndexTrait<IndexType_>::multiple)
		[[nodiscard]] bool slowContains(nmpt<Asset> asset_) const noexcept {
			using index_trait_type = IndexTrait<IndexType_>;
			return _table.contains(index_trait_type::project(asset_));
		}

		template <typename IndexType_ = Index_> requires IndexTrait<IndexType_>::unique &&
			IndexTrait<IndexType_>::multiple
		[[nodiscard]] bool slowContains(nmpt<Asset> asset_) const noexcept {
			using index_trait_type = IndexTrait<IndexType_>;
			return _table.contains(index_trait_type::project(asset_));
		}

		template <typename IndexType_ = Index_> requires (not IndexTrait<IndexType_>::unique) &&
			IndexTrait<IndexType_>::multiple
		[[nodiscard]] bool slowContains([[maybe_unused]] nmpt<Asset> asset_) const noexcept {
			// using index_trait_type = IndexTrait<IndexType_>;
			// Attention: We won't support this currently as this is only internal behaviour, unused and we expect the
			// Attention:	corresponding implementation to be "slow"
			::hg::panic();
		}

	public:
		[[nodiscard]] bool slowContains(nmpt<Asset> asset_) const noexcept override {
			return slowContains<index_type>(asset_);
		}

		/**
		 * Insert Operations
		 */
	public:
		template <typename IndexType_ = Index_> requires IndexTrait<IndexType_>::unique &&
			(!IndexTrait<IndexType_>::multiple)
		void store(nmpt<Asset> asset_) {
			using index_trait_type = IndexTrait<IndexType_>;

			#ifdef _DEBUG
			const auto result = _table.insert_or_assign(index_trait_type::project(asset_), asset_);
			::hg::assertd(result.second);
			#else
			_table.insert_or_assign(index_trait_type::project(asset_), asset_);
			#endif
		}

		template <typename IndexType_ = Index_> requires IndexTrait<IndexType_>::unique &&
			IndexTrait<IndexType_>::multiple
		void store(nmpt<Asset> asset_) {
			using index_trait_type = IndexTrait<IndexType_>;

			#ifdef _DEBUG
			const auto result = _table.insert_or_assign(index_trait_type::project(asset_), asset_);
			::hg::assertd(result.second);
			#else
			_table.insert_or_assign(index_trait_type::project(asset_), asset_);
			#endif
		}

		template <typename IndexType_ = Index_> requires (!IndexTrait<IndexType_>::unique) &&
			IndexTrait<IndexType_>::multiple
		void store(nmpt<Asset> asset_) {
			using index_trait_type = IndexTrait<IndexType_>;

			auto iter = _table.find(index_trait_type::project(asset_));
			if (iter != _table.end()) {

				// TODO: Check whether already present ?!?
				iter->second.push_back(asset_);
				return;
			}

			_table.insert_or_assign(
				index_trait_type::project(asset_),
				Vector<nmpt<Asset>>({ asset_ })
			);
		}

	public:
		void store(nmpt<Asset> asset_) override {
			store<index_type>(asset_);
		}

		/**
		 * Remove Operations
		 */
	public:
		template <typename IndexType_ = Index_> requires IndexTrait<IndexType_>::unique &&
			(!IndexTrait<IndexType_>::multiple)
		void remove(nmpt<Asset> asset_) {
			using index_trait_type = IndexTrait<IndexType_>;
			_table.erase(index_trait_type::project(asset_));
		}

		template <typename IndexType_ = Index_> requires IndexTrait<IndexType_>::unique &&
			IndexTrait<IndexType_>::multiple
		void remove(nmpt<Asset> asset_) {
			using index_trait_type = IndexTrait<IndexType_>;
			_table.erase(index_trait_type::project(asset_));
		}

		template <typename IndexType_ = Index_> requires (!IndexTrait<IndexType_>::unique) &&
			IndexTrait<IndexType_>::multiple
		void remove(nmpt<Asset> asset_) {
			using index_trait_type = IndexTrait<IndexType_>;

			auto iter = _table.find(index_trait_type::project(asset_));
			if (iter == _table.end()) {
				return;
			}

			auto subRange = std::ranges::remove(iter->second, asset_);
			iter->second.erase(subRange.end(), iter->second.end());

			if (iter->second.empty()) {
				//_table.erase(index_trait_type::project(asset_));
				_table.erase(iter);
			}
		}

	public:
		void remove(nmpt<Asset> asset_) override {
			remove<index_type>(asset_);
		}

		/**
		 * Query Operations
		 */
	public:
		/**
		 * Unique Getter
		 */

		template <typename IndexType_ = Index_> requires IndexTrait<IndexType_>::unique
		[[nodiscard]] nmpt<Asset> get(
			cref<typename IndexTrait<IndexType_>::data_type> index_
		) const noexcept {
			return _table.at(index_);
		}

		template <typename IndexType_ = Index_> requires IndexTrait<IndexType_>::unique
		[[nodiscard]] nmpt<Asset> find(
			cref<typename IndexTrait<IndexType_>::data_type> index_
		) const noexcept {
			auto iter = _table.find(index_);
			if (iter != _table.end()) {
				return iter->second;
			}
			return nullptr;
		}

		/**
		 * Unique & Multi Getter
		 */

		template <typename IndexType_ = Index_> requires IndexTrait<IndexType_>::unique &&
			IndexTrait<IndexType_>::multiple &&
			std::is_void_v<typename IndexTrait<IndexType_>::multiple_options_type>
		void get(
			cref<typename IndexTrait<IndexType_>::data_type> index_,
			_Out_ ref<Vector<nmpt<Asset>>> assets_
		) const {

			using trait_type = IndexTrait<IndexType_>;

			// TODO: Optimize

			const auto end = _table.end();
			auto iter = _table.begin();
			for (; iter != end; ++iter) {

				//auto projected = trait_type::project(*iter);
				const auto& projected = iter->first;
				const auto compared = trait_type::compare(index_, projected);

				if (compared < 0) {
					continue;
				}

				if (compared == 0) {
					assets_.push_back(iter->second);
					continue;
				}

				if (compared > 0) {
					continue;
				}
			}

		}

		template <typename IndexType_ = Index_> requires IndexTrait<IndexType_>::unique &&
			IndexTrait<IndexType_>::multiple &&
			(!std::is_void_v<typename IndexTrait<IndexType_>::multiple_options_type>)
		void get(
			cref<typename IndexTrait<IndexType_>::data_type> index_,
			typename IndexTrait<IndexType_>::multiple_options_type options_,
			_Out_ ref<Vector<nmpt<Asset>>> assets_
		) const {

			using index_trait_type = IndexTrait<IndexType_>;

			// TODO: Optimize

			const auto end = _table.end();
			auto iter = _table.begin();
			for (; iter != end; ++iter) {

				//auto projected = index_trait_type::project(*iter);
				auto projected = iter->first;
				const auto compared = index_trait_type::compare(index_, projected, options_);

				if (compared < 0) {
					continue;
				}

				if (compared == 0) {
					assets_.push_back(iter->second);
					continue;
				}

				if (compared > 0) {
					continue;
				}
			}
		}

		/**
		 * Multi Getter
		 */

		template <typename IndexType_ = Index_> requires (!IndexTrait<IndexType_>::unique) &&
			IndexTrait<IndexType_>::multiple &&
			std::is_void_v<typename IndexTrait<IndexType_>::multiple_options_type>
		void get(
			cref<typename IndexTrait<IndexType_>::data_type> index_,
			_Out_ ref<Vector<nmpt<Asset>>> assets_
		) const {
			const auto iter = _table.find(index_);
			if (iter == _table.end()) {
				return;
			}

			const auto& list = iter->second;
			assets_.insert(assets_.end(), list.begin(), list.end());
		}

		template <typename IndexType_ = Index_> requires (!IndexTrait<IndexType_>::unique) &&
			IndexTrait<IndexType_>::multiple &&
			(!std::is_void_v<typename IndexTrait<IndexType_>::multiple_options_type>)
		void get(
			cref<typename IndexTrait<IndexType_>::data_type> index_,
			typename IndexTrait<IndexType_>::multiple_options_type options_,
			_Out_ ref<Vector<nmpt<Asset>>> assets_
		) const {

			using index_trait_type = IndexTrait<IndexType_>;

			// TODO: Optimize

			const auto end = _table.end();
			auto iter = _table.begin();
			for (; iter != end; ++iter) {

				//auto projected = index_trait_type::project(*iter);
				auto projected = iter->first;
				const auto compared = index_trait_type::compare(index_, projected, options_);

				if (compared < 0) {
					continue;
				}

				if (compared == 0) {
					const auto& list = iter->second;
					assets_.insert(assets_.end(), list.begin(), list.end());
					continue;
				}

				if (compared > 0) {
					continue;
				}
			}

		}
	};
}
