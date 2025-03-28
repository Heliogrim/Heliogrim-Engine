#pragma once

#include <Heliogrim/Actor/Actor.hpp>
#include "Pool.hpp"
#include "Traits.hpp"

namespace hg::engine::acs {
	class macro_novtable ActorPoolWrapperBase {
	public:
		virtual ~ActorPoolWrapperBase() = default;

	public:
		virtual void insert(cref<ActorGuid> key_, cref<ActorInitializer> initializer_) = 0;

		virtual ptr<Actor> get(cref<ActorGuid> key_) = 0;

		virtual void erase(cref<ActorGuid> key_) = 0;
	};

	template <typename PooledType_>
	class ActorPoolWrapper final :
		public ActorPoolWrapperBase {
	public:
		using pool_type = Pool<ActorGuid, PooledType_, invalid_actor_guid>;

	public:
		ActorPoolWrapper(const ptr<pool_type> actual_) noexcept :
			_actual(actual_) {}

		~ActorPoolWrapper() override = default;

	private:
		ptr<pool_type> _actual;

	public:
		[[nodiscard]] const ptr<pool_type> pool() const noexcept {
			return _actual;
		}

	public:
		void insert(cref<ActorGuid> key_, cref<ActorInitializer> initializer_) override {
			_actual->emplace(key_, initializer_);
		}

		ptr<Actor> get(cref<ActorGuid> key_) override {
			return static_cast<ptr<Actor>>(_actual->get(key_));
		}

		void erase(cref<ActorGuid> key_) override {
			_actual->erase(key_);
		}
	};
}
