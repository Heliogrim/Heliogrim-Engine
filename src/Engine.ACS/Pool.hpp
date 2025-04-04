#pragma once

#include <functional>
#include <Engine.Common/Wrapper.hpp>

#include "Storage.hpp"

namespace hg::engine::acs {
	template <typename KeyType, typename PooledType, KeyType InvalidKey>
	class Pool {
	public:
		using storage_type = hybrid_storage<KeyType, PooledType, InvalidKey>;

		using assign_key_type = KeyType;
		using assign_value_type = PooledType;

		using assign_hasher = std::hash<assign_key_type>;

		using size_type = std::size_t;
		using hash_type = typename storage_type::hash_type;

		using value_type = Pool<KeyType, PooledType, InvalidKey>;
		using reference_type = Pool<KeyType, PooledType, InvalidKey>&;
		using const_reference_type = const Pool<KeyType, PooledType, InvalidKey>&;
		using ptr_type = Pool<KeyType, PooledType, InvalidKey>*;

		using each_lambda_const = std::function<void(const assign_key_type&, const assign_value_type&)>;
		using each_lambda = std::function<void(const assign_key_type&, assign_value_type&)>;

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

		bool canConsolidate() const {
			// TODO:
			return false;
		}

		void consolidate() {
			// TODO:
		}

		//void each(const each_lambda_const& lambda_) const {
		//    auto it = _storage.begin();
		//    const auto end = _storage.end();
		//    for (; it != _storage.end(); ++it) {
		//        lambda_(it.entity_id(), it.component());
		//    }
		//}
		//
		//void transform(const each_lambda& lambda_) {
		//    auto it = _storage.begin();
		//    const auto end = _storage.end();
		//    for (; it != _storage.end(); ++it) {
		//        lambda_(it.entity_id(), it.component());
		//    }
		//}

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
		Pool() :
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
			if (_this == nullptr) {
				_this = new value_type();
			}

			return *_this;
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
		explicit operator bool() const noexcept {
			return this != nullptr;
		}

		friend PooledType;

	public:
		/**
		 * Inserts and in-place default constructed value by the given key
		 *
		 * @author Julius
		 * @date 30.10.2020
		 *
		 * @param  key_ The key.
		 */
		template <std::same_as<PooledType> Type_ = PooledType> requires std::is_default_constructible_v<Type_>
		void insert(const KeyType& key_) {
			_storage.emplace(key_);
		}

		/**
		 * Inserts and in-place constructs a pooled type object associated with the given key
		 *
		 * @author Julius
		 * @date 30.10.2020
		 *
		 * @param key_ The associative key identifying the pooled value
		 * @param arg0_ The first parameter consumed and forwarded for inplace construction
		 * @param args_ The rest of the variadic parameter list to forward for inplace construction
		 */
		template <std::same_as<PooledType> Type_ = PooledType>
		void insert(const KeyType& key_, auto&& arg0_, auto&&... args_)
			requires std::is_constructible_v<Type_, decltype(arg0_), decltype(args_)...> {
			_storage.emplace(key_, std::forward<decltype(arg0_)>(arg0_), std::forward<decltype(args_)>(args_)...);
		}

		/**
		 * Inserts an or assign
		 *
		 * @author Julius
		 * @date 30.10.2020
		 *
		 * @param 		   key_ The key.
		 * @param [in,out] value_ The value to insert or assign.
		 *
		 * @returns A reference to a PooledType.
		 */
		template <std::same_as<PooledType> Type_ = PooledType> requires
			std::is_move_constructible_v<Type_> &&
			std::is_move_assignable_v<Type_>
		PooledType& insert_or_assign(const KeyType& key_, PooledType&& value_) {
			return _storage.insert_or_assign(key_, std::forward<PooledType>(value_));
		}

		/**
		 * Tries to emplace the given pooled type to pool constructed by args
		 *
		 * @author Julius
		 * @date 02.12.2021
		 *
		 * @tparam Args_ The packed parameter type list to construct pooled type
		 *
		 * @param key_ The key where to insert.
		 * @param args_ The packed typed parameter list to construct pooled type
		 *
		 * @returns A pointer to the keyed target element and expression whether it was newly constructed
		 */
		template <std::same_as<PooledType> Type_ = PooledType, typename... Args_> requires
			std::is_constructible_v<Type_, Args_...>
		std::pair<ptr<PooledType>, bool> emplace(cref<KeyType> key_, Args_&&... args_) {
			return _storage.emplace(key_, std::forward<Args_>(args_)...);
		}

	public:
		/**
		 * Unsafe get for const value reference
		 *	Will throw an error if key_ is not present
		 *
		 * @author Julius
		 * @date 02.11.2020
		 *
		 * @param  key_ The key.
		 *
		 * @returns A const reference to a PooledType.
		 */
		const PooledType& unsafe_get(const KeyType& key_) const {
			return _storage.unsafe_get(key_);
		}

		/**
		 * Unsafe get for const value reference
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
		 * Unsafe get for value reference
		 *	Will throw an error if key_ is not present
		 *
		 * @author Julius
		 * @date 02.11.2020
		 *
		 * @param  key_ The key.
		 *
		 * @returns A reference to a PooledType.
		 */
		PooledType& unsafe_get(const KeyType& key_) {
			return _storage.unsafe_get(key_);
		}

		/**
		 * Unsafe get for value reference
		 *  Will throw an error if key_ is not present
		 *
		 * @author Julius
		 * @date 02.11.2020
		 *
		 * @param  key_ The key.
		 * @param  hash_ The hash.
		 *
		 * @returns A reference to a PooledType.
		 */
		PooledType& unsafe_get(const assign_key_type& key_, const hash_type& hash_) {
			return _storage.unsafe_get(key_, hash_);
		}

		/**
		 * Gets a constant value pointer using the given key
		 *
		 * @author Julius
		 * @date 02.11.2020
		 *
		 * @param  key_ The key to get.
		 *
		 * @returns A pointer to a const PooledType.
		 */
		const PooledType* get(const KeyType& key_) const {
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
		 * @returns A pointer to a const PooledType.
		 */
		const PooledType* get(const assign_key_type& key_, const hash_type hash_) const {
			return _storage.get(key_, hash_);
		}

		/**
		 * Gets a value pointer using the given key
		 *
		 * @author Julius
		 * @date 02.11.2020
		 *
		 * @param  key_ The key to get.
		 *
		 * @returns A pointer to a PooledType.
		 */
		PooledType* get(const KeyType& key_) {
			return _storage.get(key_);
		}

		/**
		 * Gets a value pointer using the given key
		 *
		 * @author Julius
		 * @date 02.11.2020
		 *
		 * @param  key_ The key to get.
		 * @param  hash_ The hash.
		 *
		 * @returns A pointer to a PooledType.
		 */
		PooledType* get(const assign_key_type& key_, const hash_type hash_) {
			return _storage.get(key_, hash_);
		}

	public:
		/**
		 * Erase a value from this pool by given key
		 *
		 * @author Julius
		 * @date 30.10.2021
		 *
		 * @param   key_ The key to erase
		 */
		void erase(cref<assign_key_type> key_) {
			_storage.erase(key_);
		}

		/**/
		[[nodiscard]] const auto& __get_storage() const noexcept {
			return _storage;
		}

		/**/
	};

	template <typename KeyType_, typename PooledType_, KeyType_ InvalidKey_>
	typename Pool<KeyType_, PooledType_, InvalidKey_>::ptr_type Pool<KeyType_, PooledType_, InvalidKey_>::_this =
		nullptr;
}
