﻿#pragma once

#include "__macro.hpp"
#include "Wrapper.hpp"

#include "Concurrent/SharedMemoryReference.hpp"
#include "Meta/Concept.hpp"
#include "Meta/Type.hpp"

namespace hg {
	template <CompleteType Ty, class... Args>
	ptr<Ty> make_ptr(Args&&... args_) {
		return new Ty(std::forward<Args>(args_)...);
	}

	/* Unique Pointer (STD) */

	template <CompleteType Ty_, class... Args_> requires std::is_constructible_v<Ty_, Args_&&...>
	[[nodiscard]] uptr<Ty_> make_uptr(Args_&&... args_) {
		return std::make_unique<Ty_>(std::forward<Args_>(args_)...);
	}

	template <CompleteType Ty_, class... Args_> requires
		(not std::is_constructible_v<Ty_, Args_...>) &&
		(std::is_aggregate_v<Ty_>)
	[[nodiscard]] uptr<Ty_> make_uptr(Args_&&... args_) {
		return uptr<Ty_>(new Ty_ { std::forward<Args_>(args_)... });
	}

	/*
	template <class Ty, class... Args>
	uptr<Ty> make_uptr(Args&&... args_) {
	    return std::make_unique<Ty>(std::forward<Args>(args_)...);
	}
	 */

	/* Shared Pointer (STD) */

	template <CompleteType Ty_, class... Args_> requires std::is_constructible_v<Ty_, Args_...>
	sptr<Ty_> make_sptr(Args_&&... args_) {
		return std::make_shared<Ty_>(std::forward<Args_>(args_)...);
	}

	/*
	template <class Ty_, class... Args_> requires std::is_constructible_v<sptr<Ty_>, Args_...>
	sptr<Ty_> make_sptr(Args_&&... args_) {
	    return std::make_shared<Ty_>(std::forward<Args_>(args_)...);
	}
	 */

	/* Shared Memory References */

	template <CompleteType Ty>
	smr<Ty> make_smr(mref<uptr<Ty>> unique_) {
		return (new SharedMemoryReferenceCtrlBlock<Ty>(unique_.release()))->acq();
	}

	template <CompleteType Ty> requires decayed_as<Ty, Ty>
	smr<Ty> make_smr(mref<ptr<Ty>> ptr_) {
		return (new SharedMemoryReferenceCtrlBlock<Ty>(std::move(ptr_)))->acq();
	}

	template <CompleteType Ty_, class Tx_> requires
		(not std::is_same_v<Ty_, Tx_>) &&
		(std::is_nothrow_convertible_v<ptr<Tx_>, ptr<Ty_>>)
	smr<Ty_> make_smr(mref<ptr<Tx_>> ptrTx_) {
		return (new SharedMemoryReferenceCtrlBlock<Tx_>(std::move(ptrTx_)))->acq().template into<Ty_>();
	}

	template <CompleteType Ty, class... Args> requires std::is_constructible_v<Ty, Args...>
	smr<Ty> make_smr(Args&&... args_) {
		return (new SharedMemoryReferenceCtrlBlock<Ty>(
			// TODO: Check whether we get problems with types which differ between aggregate and list construction e.g std::vector<...>
			new Ty { std::forward<Args>(args_)... }
		))->acq();
	}

	template <CompleteType Type_>
	smr<Type_> make_smr(Type_&& initial_) requires std::is_move_constructible_v<Type_> {
		return (new SharedMemoryReferenceCtrlBlock<Type_>(new Type_(std::forward<Type_>(initial_))))->acq();
	}
}
