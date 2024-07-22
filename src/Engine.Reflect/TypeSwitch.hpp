#pragma once

#include <functional>
#include <Engine.Common/Functional/Function.hpp>

#include "Inherit/Concept.hpp"
#include "Meta/TypeId.hpp"
#include "Template/QueryTypeId.hpp"

namespace hg {
	namespace {
		template <typename... List_>
		struct pick;

		template <>
		struct pick<> {
			using type = void;
		};

		template <typename Type_, typename... Rest_>
		struct pick<Type_, Rest_...> {
			using type = Type_;
		};

		template <typename Ret_, typename Arg_, typename... Rest_>
		[[maybe_unused]] Arg_ __decompose_fx_args(tuple_fnc<Ret_, Arg_, Rest_...>);

		template <typename Fx_, typename Ret_, typename Arg_, typename... Rest_>
		[[maybe_unused]] Arg_ __decompose_fx_args(bound_tuple_fnc<Fx_, Ret_, Arg_, Rest_...>);

		template <typename Fx_, typename Ret_, typename Arg_, typename... Rest_>
		[[maybe_unused]] Arg_ __decompose_fx_args(const_bound_tuple_fnc<Fx_, Ret_, Arg_, Rest_...>);

		template <typename Fx_>
		[[maybe_unused]] decltype(__decompose_fx_args(&Fx_::operator())) __decompose_fx_args(Fx_);

		template <typename Callable_>
		using decompose_subject_type = decltype(__decompose_fx_args(std::declval<Callable_>()));

		template <typename Callable_>
		struct decompose {
			using subject_type = decompose_subject_type<Callable_>;
			using return_type = decltype(std::declval<Callable_>()(std::declval<subject_type>()));
		};

		template <typename Subject_, typename ReturnType_>
		struct decompose<std::function<ReturnType_(Subject_)>> {
			using subject_type = Subject_;
			using return_type = ReturnType_;
		};

		template <typename ProjectFn_>
		using project_type = typename decompose<ProjectFn_>::subject_type;
	}

	template <
		class Value_,
		typename ReturnType_,
		typename Fn_,
		typename... Rest_
	> requires (ClassHasMeta<std::remove_const_t<Value_>>)
	ReturnType_ switchTypeImpl(
		Value_* value_,
		Fn_&& fn_,
		Rest_&&... rest_
	) {

		using check_type = std::remove_pointer_t<typename decompose<Fn_>::subject_type>;

		if (value_->getMetaClass()->typeId().data == reflect::typeId<std::remove_const_t<check_type>>().data) {
			return std::forward<Fn_>(fn_)(static_cast<check_type*>(value_));
		}
		return switchTypeImpl<Value_, ReturnType_, Rest_...>(
			value_,
			std::forward<Rest_>(rest_)...
		);
	}

	template <
		class Value_,
		typename ReturnType_,
		typename Fn_
	> requires (ClassHasMeta<std::remove_const_t<Value_>>) && (not std::is_void_v<ReturnType_>)
	ReturnType_ switchTypeImpl(
		Value_* value_,
		Fn_&& fn_
	) {

		using check_type = std::remove_pointer_t<typename decompose<Fn_>::subject_type>;

		if (value_->getMetaClass()->typeId().data == reflect::typeId<std::remove_const_t<check_type>>().data) {
			return std::forward<Fn_>(fn_)(static_cast<check_type*>(value_));
		}

		return ReturnType_ {};
	}

	template <
		class Value_,
		typename ReturnType_,
		typename Fn_
	> requires (ClassHasMeta<std::remove_const_t<Value_>>) && (std::is_void_v<ReturnType_>)
	void switchTypeImpl(
		Value_* value_,
		Fn_&& fn_
	) {

		using check_type = std::remove_pointer_t<typename decompose<Fn_>::subject_type>;

		if (value_->getMetaClass()->typeId().data == reflect::typeId<std::remove_const_t<check_type>>().data) {
			return std::forward<Fn_>(fn_)(static_cast<check_type*>(value_));
		}
	}

	template <ClassHasMeta Ty_, typename Fn_, typename... Rfn_>
	typename decompose<Fn_>::return_type switchType(Ty_* value_, Fn_&& fn_, Rfn_&&... rest_) {

		using return_type = typename decompose<Fn_>::return_type;

		return switchTypeImpl<Ty_, return_type, Fn_, Rfn_...>(
			value_,
			std::forward<Fn_>(fn_),
			std::forward<Rfn_>(rest_)...
		);
	}

	template <ClassHasMeta Ty_, typename Fn_, typename... Rfn_>
	typename decompose<Fn_>::return_type switchType(const Ty_* value_, Fn_&& fn_, Rfn_&&... rest_) {

		using return_type = typename decompose<Fn_>::return_type;

		return switchTypeImpl<const Ty_, return_type, Fn_, Rfn_...>(
			value_,
			std::forward<Fn_>(fn_),
			std::forward<Rfn_>(rest_)...
		);
	}

	/*
	template <ClassHasMeta Ty_, reflect::TypeQueryable ...Types_, typename ReturnType_>
	ReturnType_ switchType(Ty_* value_, std::function<ReturnType_(Types_*)>... fn_) {
	    return switchTypeImpl<Ty_, ReturnType_, Types_...>(
	        value_,
	        std::forward<std::function<ReturnType_(Types_*)>>(fn_)...
	    );
	}
	*/
}
