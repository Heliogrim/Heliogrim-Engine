#pragma once

#include <functional>

#include "Traits.hpp"
#include "Storage.hpp"

namespace ember::engine::ecs {
    template <IsComponent ComponentType>
    class pool {
    public:
        using storage_type = typename hybrid_storage<ComponentType>::this_type;

        using assign_key_type = entity_guid;
        using assign_value_type = ComponentType;

        using assign_hasher = _STD hash<assign_key_type>;

        using size_type = _STD size_t;
        using hash_type = typename storage_type::hash_type;

        using value_type = pool<ComponentType>;
        using reference_type = pool<ComponentType>&;
        using const_reference_type = const pool<ComponentType>&;
        using ptr_type = pool<ComponentType>*;

        using each_lambda_const = _STD function<void(const assign_key_type&, const assign_value_type&)>;
        using each_lambda = _STD function<void(const assign_key_type&, assign_value_type&)>;

        /**
         * Gets the size
         *
         * @author Julius
         * @date 30.10.2020
         *
         * @returns A size_t.
         */
        [[nodiscard]] size_type size() const noexcept {
            return _storage.size();
        }

        const bool canConsolidate() const {
            // TODO:
            return false;
        }

        void consolidate() {
            // TODO:
        }

        /**
         * Will iterator over each element and invoke the given lambda
         *
         * @author Julius
         * @date 30.10.2020
         *
         * @param  lambda_ The lambda.
         */
        void each(const each_lambda_const& lambda_) const {
            auto it = _storage.begin();
            const auto end = _storage.end();
            for (; it != _storage.end(); ++it) {
                lambda_(it.entity_id(), it.component());
            }
        }

        // TODO: void each(const size_t pageIndex_, const each_lambda_const& lambda_) const {}

        /*
        template <typename Task, _STD enable_if_t<_STD is_function_v<Task>>>
        vector<Task> unsequenced_each(const each_lambda_const& lambda_) const {
            vector<Task> batch {};
            batch.reserve(_storage.page_count());

            for (const auto& storage : _sparse) {
                batch.push_back([iter_ = storage.begin(), &lambda_]() {
                    while (iter_.next()) {
                        lambda_(iter_.entity_id(), iter_.component());
                    }
                });
            }
        }
         */

        /**
         * Transforms the given lambda
         *
         * @author Julius
         * @date 30.10.2020
         *
         * @param  lambda_ The lambda.
         */
        void transform(const each_lambda& lambda_) {
            auto it = _storage.begin();
            const auto end = _storage.end();
            for (; it != _storage.end(); ++it) {
                lambda_(it.entity_id(), it.component());
            }
        }

        // TODO: void transform(const size_t pageIndex_, const each_lambda& lambda_) {}

        /*
        template <typename Task, _STD enable_if_t<_STD is_function_v<Task>>>
        vector<Task> unsequenced_transform(const each_lambda& lambda_) {
            vector<Task> batch {};
            batch.reserve(_sparse.size());

            for (auto& storage : _sparse) {
                batch.push_back([iter_ = storage.begin(), &lambda_]() {
                    lambda_(iter_.entity_id(), iter_.component());
                });
            }

            return batch;
        }
         */

    private:
        static ptr_type _this;

        /** The storage */
        storage_type _storage;

        /**
         * Default constructor
         *
         * @author Julius
         * @date 22.10.2020
         */
        pool() :
            _storage() {
            _this = this;
        }

    public:
        /**
         * Destroys the static instance
         *
         * @author Julius
         * @date 22.10.2020
         */
        static void destroy() {
            delete value_type::_this;
        }

        /**
         * Gets or Creates the static instance
         *
         * @author Julius
         * @date 22.10.2020
         *
         * @returns The reference to the static instance
         */
        static reference_type getOrCreate() {
            return *(new value_type());
        }

        /**
         * Gets the static instance
         *
         * @author Julius
         * @date 22.10.2020
         *
         * @returns The reference to the static instance
         */
        static reference_type get() {
            return *(_this);
        }

        /**
         * Cast that converts the given to a bool
         *
         * @author Julius
         * @date 06.11.2020
         *
         * @returns The result of the operation.
         */
        operator bool() const {
            return this != nullptr;
        }

        friend ComponentType;

    public:
        /**
         * Component Operations
         */

        /**
         * Inserts and in-place default constructed component by the given key
         *
         * @author Julius
         * @date 30.10.2020
         *
         * @param  key_ The key.
         */
        void insert(const entity_guid& key_) {
            _storage.emplace(key_);
        }

        /**
         * Inserts and in-place move constructs component by the given key
         *
         * @author Julius
         * @date 30.10.2020
         *
         * @param 		   key_ The key.
         * @param [in,out] component_ The component.
         */
        void insert(const entity_guid& key_, ComponentType&& component_) {
            _storage.emplace(key_, _STD forward<ComponentType>(component_));
        }

        /**
         * Inserts an or assign
         *
         * @author Julius
         * @date 30.10.2020
         *
         * @param 		   key_ The key.
         * @param [in,out] component_ The component.
         *
         * @returns A reference to a ComponentType.
         */
        ComponentType& insert_or_assign(const entity_guid& key_, ComponentType&& component_) {
            return _storage.insert_or_assign(key_, _STD forward<ComponentType>(component_));
        }

    public:
        /**
         * Unsafe get for const component reference
         *	Will throw an error if key_ is not present
         *
         * @author Julius
         * @date 02.11.2020
         *
         * @param  key_ The key.
         *
         * @returns A reference to a const ComponentType.
         */
        const ComponentType& unsafe_get(const entity_guid& key_) const {
            return _storage.unsafe_get(key_);
        }

        /**
         * Unsafe get for const component reference
         *	Will throw an error if key_ is not present
         *
         * @author Julius
         * @date 09.11.2020
         *
         * @param  key_ The key.
         * @param  hash_ The hash.
         *
         * @returns A reference to a const assign_value_type.
         */
        const assign_value_type& unsafe_get(const assign_key_type& key_, const hash_type hash_) const {
            return _storage.unsafe_get(key_, hash_);
        }

        /**
         * Unsafe get for component reference
         *	Will throw an error if key_ is not present
         *
         * @author Julius
         * @date 02.11.2020
         *
         * @param  key_ The key.
         *
         * @returns A reference to a ComponentType.
         */
        ComponentType& unsafe_get(const entity_guid& key_) {
            return _storage.unsafe_get(key_);
        }

        /**
         * Unsafe get for component reference
         *  Will throw an error if key_ is not present
         *
         * @author Julius
         * @date 02.11.2020
         *
         * @param  key_ The key.
         * @param  hash_ The hash.
         *
         * @returns A reference to a ComponentType.
         */
        ComponentType& unsafe_get(const assign_key_type& key_, const hash_type& hash_) {
            return _storage.unsafe_get(key_, hash_);
        }

        /**
         * Gets a constant component pointer using the given key
         *
         * @author Julius
         * @date 02.11.2020
         *
         * @param  key_ The key to get.
         *
         * @returns A pointer to a const ComponentType.
         */
        const ComponentType* get(const entity_guid& key_) const {
            return _storage.get(key_);
        }

        /**
         * Gets a constant component pointer using the given key
         *
         * @author Julius
         * @date 02.11.2020
         *
         * @param  key_ The key to get.
         * @param  hash_ The hash.
         *
         * @returns A pointer to a const ComponentType.
         */
        const ComponentType* get(const assign_key_type& key_, const hash_type hash_) const {
            return _storage.get(key_, hash_);
        }

        /**
         * Gets a component pointer using the given key
         *
         * @author Julius
         * @date 02.11.2020
         *
         * @param  key_ The key to get.
         *
         * @returns A pointer to a ComponentType.
         */
        ComponentType* get(const entity_guid& key_) {
            return _storage.get(key_);
        }

        /**
         * Gets a component pointer using the given key
         *
         * @author Julius
         * @date 02.11.2020
         *
         * @param  key_ The key to get.
         * @param  hash_ The hash.
         *
         * @returns A pointer to a ComponentType.
         */
        ComponentType* get(const assign_key_type& key_, const hash_type hash_) {
            return _storage.get(key_, hash_);
        }

    public:
        /**
         * Erase a component from this pool by given key
         *
         * @author Julius
         * @date 30.10.2021
         *
         * @param   key_ The key to erase
         */
        void erase(cref<assign_key_type> key_) {
            _storage.erase(key_);
        }
    };

    template <IsComponent ComponentType>
    typename pool<ComponentType>::ptr_type pool<ComponentType>::_this = nullptr;
}
