#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/RobinMap.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <map>

#include "../__fwd.hpp"

#include "Index.hpp"
#include "IndexTrait.hpp"

namespace hg::engine::assets::system {
    class IndexTableBase {
    public:
        template <typename>
        friend class IndexTable;

    public:
        using this_type = IndexTableBase;

    private:
        IndexTableBase(mref<string> uniqueName_) :
            _uniqueName(_STD move(uniqueName_)) {}

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
        virtual void clear() = 0;

        virtual void store(const non_owning_rptr<Asset> asset_) = 0;

        virtual void remove(const non_owning_rptr<Asset> asset_) = 0;
    };

    template <typename Index_>
    class IndexTable :
        public IndexTableBase {
    public:
        using this_type = IndexTable<Index_>;

        using index_type = Index_;
        using trait_type = IndexTrait<index_type>;

        using allocator_type = _STD allocator<typename trait_type::data_type>;
        using allocator_traits = _STD allocator_traits<allocator_type>;

    protected:
        IndexTable(mref<string> uniqueName_) :
            IndexTableBase(_STD move(uniqueName_)) {}

    public:
        ~IndexTable() override = default;
    };

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

        template <bool TestUnique_, bool TestMultiple_>
        struct AutoMappingSelector;

        template <>
        struct AutoMappingSelector<true, false> {
            using table_type = RobinMap<
                typename trait_type::data_type,
                non_owning_rptr<Asset>,
                typename trait_type::hash_type
            >;
        };

        template <>
        struct AutoMappingSelector<true, true> {
            using table_type = _STD map<
                typename trait_type::data_type,
                non_owning_rptr<Asset>,
                typename trait_type::relation_type
            >;
        };

        template <>
        struct AutoMappingSelector<false, true> {
            using table_type = _STD map<
                typename trait_type::data_type,
                Vector<non_owning_rptr<Asset>>,
                typename trait_type::relation_type
            >;
        };

        template <>
        struct AutoMappingSelector<false, false> {
            static_assert(trait_type::unique || trait_type::multiple);
        };

        /**/

        using auto_type = AutoMappingSelector<trait_type::unique, trait_type::multiple>;
        using auto_table_type = typename auto_type::table_type;

    public:
        AutoIndexTable(mref<string> uniqueName_) :
            IndexTable<Index_>(_STD move(uniqueName_)),
            _table() {
            static_assert(!trait_type::dynamic);
        }

        ~AutoIndexTable() override = default;

    private:
        auto_table_type _table;

    public:
        [[nodiscard]] auto tableKeys() const {
            return _STD ranges::views::keys(_table);
        }

        [[nodiscard]] auto tableValues() const {
            return _STD ranges::views::values(_table);
        }

    public:
        void clear() override {
            _table.clear();
        }

        /**
         * Insert Operations
         */
    public:
        template <typename IndexType_ = Index_> requires IndexTrait<IndexType_>::unique &&
            (!IndexTrait<IndexType_>::multiple)
        void store(const non_owning_rptr<Asset> asset_) {
            using index_trait_type = IndexTrait<IndexType_>;

            #ifdef _DEBUG
            const auto result = _table.insert_or_assign(index_trait_type::project(asset_), asset_);
            assert(result.second);
            #else
            _table.insert_or_assign(index_trait_type::project(asset_), asset_);
            #endif
        }

        template <typename IndexType_ = Index_> requires IndexTrait<IndexType_>::unique &&
            IndexTrait<IndexType_>::multiple
        void store(const non_owning_rptr<Asset> asset_) {
            using index_trait_type = IndexTrait<IndexType_>;

            #ifdef _DEBUG
            const auto result = _table.insert_or_assign(index_trait_type::project(asset_), asset_);
            assert(result.second);
            #else
            _table.insert_or_assign(index_trait_type::project(asset_), asset_);
            #endif
        }

        template <typename IndexType_ = Index_> requires (!IndexTrait<IndexType_>::unique) &&
            IndexTrait<IndexType_>::multiple
        void store(const non_owning_rptr<Asset> asset_) {
            using index_trait_type = IndexTrait<IndexType_>;

            auto iter = _table.find(index_trait_type::project(asset_));
            if (iter != _table.end()) {

                // TODO: Check whether already present ?!?
                iter->second.push_back(asset_);
                return;
            }

            _table.insert_or_assign(
                index_trait_type::project(asset_),
                Vector<non_owning_rptr<Asset>>({ asset_ })
            );
        }

    public:
        void store(const non_owning_rptr<Asset> asset_) override {
            store<index_type>(asset_);
        }

        /**
         * Remove Operations
         */
    public:
        template <typename IndexType_ = Index_> requires IndexTrait<IndexType_>::unique &&
            (!IndexTrait<IndexType_>::multiple)
        void remove(const non_owning_rptr<Asset> asset_) {
            using index_trait_type = IndexTrait<IndexType_>;
            _table.erase(index_trait_type::project(asset_));
        }

        template <typename IndexType_ = Index_> requires IndexTrait<IndexType_>::unique &&
            IndexTrait<IndexType_>::multiple
        void remove(const non_owning_rptr<Asset> asset_) {
            using index_trait_type = IndexTrait<IndexType_>;
            _table.erase(index_trait_type::project(asset_));
        }

        template <typename IndexType_ = Index_> requires (!IndexTrait<IndexType_>::unique) &&
            IndexTrait<IndexType_>::multiple
        void remove(const non_owning_rptr<Asset> asset_) {
            using index_trait_type = IndexTrait<IndexType_>;

            auto iter = _table.find(index_trait_type::project(asset_));
            if (iter == _table.end()) {
                return;
            }

            auto subRange = _STD ranges::remove(iter->second, asset_);
            iter->second.erase(subRange.end(), iter->second.end());

            if (iter->second.empty()) {
                //_table.erase(index_trait_type::project(asset_));
                _table.erase(iter);
            }
        }

    public:
        void remove(const non_owning_rptr<Asset> asset_) override {
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
        [[nodiscard]] non_owning_rptr<Asset> get(
            cref<typename IndexTrait<IndexType_>::data_type> index_
        ) const noexcept {
            return _table.at(index_);
        }

        template <typename IndexType_ = Index_> requires IndexTrait<IndexType_>::unique
        [[nodiscard]] non_owning_rptr<Asset> find(
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
            _STD is_void_v<typename IndexTrait<IndexType_>::multiple_options_type>
        void get(
            cref<typename IndexTrait<IndexType_>::data_type> index_,
            _Out_ ref<Vector<non_owning_rptr<Asset>>> assets_
        ) const {

            using trait_type = IndexTrait<IndexType_>;

            // TODO: Optimize

            const auto end = _table.end();
            auto iter = _table.begin();
            for (; iter != end; ++iter) {

                //auto projected = trait_type::project(*iter);
                const auto& projected = iter->first;
                const auto compared = trait_type::template compare(index_, projected);

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
            (!_STD is_void_v<typename IndexTrait<IndexType_>::multiple_options_type>)
        void get(
            cref<typename IndexTrait<IndexType_>::data_type> index_,
            typename IndexTrait<IndexType_>::multiple_options_type options_,
            _Out_ ref<Vector<non_owning_rptr<Asset>>> assets_
        ) const {

            using index_trait_type = IndexTrait<IndexType_>;

            // TODO: Optimize

            const auto end = _table.end();
            auto iter = _table.begin();
            for (; iter != end; ++iter) {

                //auto projected = index_trait_type::project(*iter);
                auto projected = iter->first;
                const auto compared = index_trait_type::template compare(index_, projected, options_);

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
            _STD is_void_v<typename IndexTrait<IndexType_>::multiple_options_type>
        void get(
            cref<typename IndexTrait<IndexType_>::data_type> index_,
            _Out_ ref<Vector<non_owning_rptr<Asset>>> assets_
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
            (!_STD is_void_v<typename IndexTrait<IndexType_>::multiple_options_type>)
        void get(
            cref<typename IndexTrait<IndexType_>::data_type> index_,
            typename IndexTrait<IndexType_>::multiple_options_type options_,
            _Out_ ref<Vector<non_owning_rptr<Asset>>> assets_
        ) const {

            using index_trait_type = IndexTrait<IndexType_>;

            // TODO: Optimize

            const auto end = _table.end();
            auto iter = _table.begin();
            for (; iter != end; ++iter) {

                //auto projected = index_trait_type::project(*iter);
                auto projected = iter->first;
                const auto compared = index_trait_type::template compare(index_, projected, options_);

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
