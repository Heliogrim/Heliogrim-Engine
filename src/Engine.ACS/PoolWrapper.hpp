#pragma once

#include "Pool.hpp"
#include "Traits.hpp"

namespace hg::engine::acs {
	class macro_novtable PoolWrapperBase {
	public:
		virtual ~PoolWrapperBase() = default;

		virtual void insert(cref<ActorGuid> key_) = 0;

		virtual ptr<void> get(cref<ActorGuid> key_) = 0;

		virtual void erase(cref<ActorGuid> key_) = 0;
	};

	template <typename PooledType_>
	class PoolWrapper final :
		public PoolWrapperBase {
	public:
		using pool_type = Pool<ActorGuid, PooledType_, invalid_actor_guid>;

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
		void insert(cref<ActorGuid> key_) override {
			if constexpr (std::is_default_constructible_v<typename pool_type::assign_value_type>) {
				_actual->insert(key_);
			}
		}

		ptr<void> get(cref<ActorGuid> key_) override {
			return _actual->get(key_);
		}

		void erase(cref<ActorGuid> key_) override {
			_actual->erase(key_);
		}
	};
}
