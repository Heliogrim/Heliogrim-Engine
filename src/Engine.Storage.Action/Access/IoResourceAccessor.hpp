#pragma once

#include <utility>
#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Optional.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Functional/Function.hpp>
#include <Engine.Common/Memory/Address.hpp>

#include "IoResourceAccessorBase.hpp"
#include "IoResourceBase.hpp"

namespace hg::engine::storage {
	template <typename Type_>
	class IoResourceAccessor final :
		public IoResourceAccessorBase {
	public:
		using this_type = IoResourceAccessor<Type_>;
		using base_type = IoResourceAccessorBase;

		using resource_type = Type_;
		using holder_type = IoResourceBase;

		template <class Obj_ = holder_type>
		using release_type = const_bound_tuple_fnc<Obj_, void, ref<this_type>>;

	public:
		constexpr IoResourceAccessor() noexcept :
			IoResourceAccessorBase(),
			_holder(nullptr),
			_releaseFn(nullptr),
			_resource(None) {}

		template <class SpecificHolder_>
		constexpr explicit IoResourceAccessor(
			cref<SpecificHolder_> holder_,
			release_type<SpecificHolder_> releaseFn_,
			auto&&... args_
		) :
			IoResourceAccessorBase(),
			_holder(std::addressof(holder_)),
			_releaseFn(static_cast<release_type<>>(releaseFn_)),
			_resource(::hg::forward<decltype(args_)>(args_)...) {}

		constexpr IoResourceAccessor(IoResourceAccessor&& other_) noexcept :
			_holder(std::exchange(other_._holder, nullptr)),
			_releaseFn(std::exchange(other_._releaseFn, nullptr)),
			_resource(std::exchange(other_._resource, None)) {}

		~IoResourceAccessor() noexcept override {
			reset();
		}

	public:
		constexpr ref<this_type> operator=(mref<this_type> other_) noexcept {
			if (std::addressof(other_) != this) {
				reset();
				_holder = std::exchange(other_._holder, nullptr);
				_releaseFn = std::exchange(other_._releaseFn, nullptr);
				_resource = std::exchange(other_._resource, None);
			}
			return *this;
		}

	private:
		ptr<const holder_type> _holder;
		release_type<> _releaseFn;
		Opt<ref<resource_type>> _resource;

	private:
		void scopedLock() {
			std::unreachable();
		}

		void scopedRelease() {
			((*_holder).*_releaseFn)(*this);
			_holder = nullptr;
			_resource = None;
		}

	public:
		[[nodiscard]] constexpr bool valid() const noexcept {
			return _holder != nullptr && _resource.has_value();
		}

		[[nodiscard]] constexpr bool invalid() const noexcept {
			return _holder == nullptr || not _resource.has_value();
		}

		[[nodiscard]] constexpr bool owns() const noexcept {
			return _resource.has_value();
		}

		[[nodiscard]] cref<IoResourceBase> holder() const {
			::hg::assertrt(_holder);
			return *_holder;
		}

		void reset() {
			if (owns()) {
				scopedRelease();
			}
		}

	public:
		[[nodiscard]] decltype(auto) get() const {
			::hg::assertrt(valid());
			if constexpr (std::is_const_v<resource_type>) {
				return *_resource;
			} else {
				using mutable_type = std::add_lvalue_reference_t<resource_type>;
				return const_cast<mutable_type>(*_resource);
			}
		}

		[[nodiscard]] decltype(auto) get() {
			::hg::assertrt(valid());
			return *_resource;
		}

	public:
		[[nodiscard]] decltype(auto) operator*() const {
			return get();
		}

		[[nodiscard]] decltype(auto) operator*() {
			return get();
		}

		[[nodiscard]] constexpr decltype(auto) operator->() const {
			if constexpr (std::is_reference_v<decltype(this->get())>) {
				return std::addressof(get());
			} else {
				return hg::unfancy(get());
			}
		}

		[[nodiscard]] constexpr decltype(auto) operator->() {
			if constexpr (std::is_reference_v<decltype(this->get())>) {
				return std::addressof(get());
			} else {
				return hg::unfancy(get());
			}
		}
	};
}
