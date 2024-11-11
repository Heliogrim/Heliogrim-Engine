#pragma once

#include <Engine.Common/Expected.hpp>
#include <Engine.Common/Variant.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Pedantic.Resource/Resource.hpp>
#include <Engine.Reflect.Utils/Error/IllegalArgumentError.hpp>
#include <Engine.Reflect.Utils/Error/IllegalTypeError.hpp>
#include <Engine.Reflect.Utils/With/WithType.hpp>

#include "../Query.hpp"
#include "../Access/IoResourceAccessor.hpp"
#include "../Access/IoResourceAccessorBase.hpp"

namespace hg::engine::storage {
	class ExtendedStorageSystem;
}

namespace hg::engine::storage {
	template <class Type_>
	class IoQueryGuard {
	public:
		using this_type = IoQueryGuard<Type_>;
		using query_type = Query;

		/**/

		using accessor_type = IoResourceAccessor<Type_>;
		using error_type = Variant<
			std::invalid_argument, std::out_of_range, std::runtime_error,
			AccessError, IllegalArgumentError, IllegalTypeError
		>;

		template <typename Fn_>
		using result_type = Result<
			std::invoke_result_t<Fn_, std::add_lvalue_reference_t<Type_>>,
			error_type
		>;

		/**/

		template <typename Fn_>
		using concrete_carry_type = meta::WithType<UniquePtr<
			std::invoke_result_t<Fn_, std::add_lvalue_reference_t<Type_>>,
			void(*)(void*)
		>>;

	public:
		constexpr explicit IoQueryGuard(_Inout_ ref<ExtendedStorageSystem> system_, auto&&... args_) noexcept :
			_system(system_),
			_query(std::forward<decltype(args_)>(args_)...) {}

		IoQueryGuard(const this_type&) = delete;

		IoQueryGuard(this_type&&) = delete;

		constexpr ~IoQueryGuard() noexcept {
			if (_query != nullptr) {
				_query->cleanup();
			}
		}

	public:
		ref<this_type> operator=(cref<this_type>) = delete;

		ref<this_type> operator=(mref<this_type>) = delete;

	private:
		ref<ExtendedStorageSystem> _system;
		uptr<query_type> _query;

	private:
		template <CompleteType SystemType_ = ExtendedStorageSystem>
		[[nodiscard]] decltype(auto) exec(auto&&... args_) {
			return static_cast<ref<SystemType_>>(_system).execQuery(*_query, ::hg::forward<decltype(args_)>(args_)...);
		}

	public:
		template <typename Fn_> requires (not std::is_void_v<typename result_type<Fn_>::value_type>)
		[[nodiscard]] result_type<Fn_> operator()(Fn_&& fn_) {

			using result_value_type = typename result_type<Fn_>::value_type;

			/**/

			auto wrapper = [_fn = ::hg::forward<Fn_>(fn_)](cref<IoResourceAccessorBase> accessor_) -> Query::exec_valreturn_type {

				const auto& accessor = static_cast<cref<accessor_type>>(accessor_);

				auto ret = concrete_carry_type<Fn_>::from(
					UniquePtr<result_value_type, void(*)(void*)> {
						std::make_unique<result_value_type>(std::invoke(_fn, *accessor)).release(),
						[](void* obj_) {
							delete static_cast<ptr<result_value_type>>(obj_);
						}
					}
				);
				return ::hg::move(*reinterpret_cast<ptr<Query::exec_valreturn_type>>(&ret));
			};

			/**/

			// Problem: Seams like std::function<...> is allowed to discard the return type...
			// Problem:		This would result in an ambiguous ADL with `execQuery(...)`
			auto result = exec(
				std::function<Query::exec_valreturn_type(cref<IoResourceAccessorBase>)> {
					::hg::move(wrapper)
				}
			);
			// TODO: auto result = exec(::hg::move(wrapper));
			if (not result.has_value()) {
				return Unexpected { ::hg::move(result).error() };
			}

			/**/

			auto& typed = reinterpret_cast<ref<concrete_carry_type<Fn_>>>(result.value());
			::hg::assertrt(typed.valid());

			/**/

			return Expected<result_value_type> { ::hg::move(*typed.value) };
		}

		template <typename Fn_> requires std::is_void_v<typename result_type<Fn_>::value_type>
		[[nodiscard]] result_type<Fn_> operator()(Fn_&& fn_) {

			auto wrapper = [_fn = ::hg::forward<Fn_>(fn_)](cref<IoResourceAccessorBase> accessor_) -> void {
				const auto& accessor = static_cast<cref<accessor_type>>(accessor_);
				std::invoke(_fn, *accessor);
			};

			auto result = exec(::hg::move(wrapper));

			if (result.has_value()) {
				return {};
			}

			return Unexpected { ::hg::move(result).error() };
		}
	};
}
