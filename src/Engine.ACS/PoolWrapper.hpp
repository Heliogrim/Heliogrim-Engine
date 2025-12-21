#pragma once

#include "Pool.hpp"
#include "Traits.hpp"

namespace hg::engine::acs {
	template <typename KeyType_>
	class macro_novtable PoolWrapperBase {
	public:
		virtual ~PoolWrapperBase() = default;

		virtual void insert(ref<const KeyType_> key_) = 0;

		virtual ptr<void> get(ref<const KeyType_> key_) = 0;

		virtual void erase(ref<const KeyType_> key_) = 0;
	};

	template <typename PooledType_, typename KeyType_, KeyType_ InvalidKey_>
	class PoolWrapper final :
		public PoolWrapperBase<KeyType_> {
	public:
		using pool_type = Pool<KeyType_, PooledType_, InvalidKey_>;

	public:
		PoolWrapper(const ptr<pool_type> actual_) noexcept :
			_actual(actual_) {}

		~PoolWrapper() override = default;

	private:
		ptr<pool_type> _actual;

	public:
		[[nodiscard]] const ptr<pool_type> pool() const noexcept {
			return _actual;
		}

	public:
		void insert(ref<const KeyType_> key_) override {
			if constexpr (std::is_default_constructible_v<typename pool_type::assign_value_type>) {
				_actual->insert(key_);
			}
		}

		ptr<void> get(ref<const KeyType_> key_) override {
			return _actual->get(key_);
		}

		void erase(ref<const KeyType_> key_) override {
			_actual->erase(key_);
		}
	};
}
