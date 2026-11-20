#pragma once

#include "AtomicRefCounted.hpp"
#include "AtomicRefCountedIntrusive.hpp"
#include "RefCounted.hpp"
#include "RefCountedIntrusive.hpp"
#include "../Concurrent/SharedMemoryReference.hpp"

namespace hg {
	/**
	 * Reference Counted < Type_ >
	 */
	template <typename Type_>
	using Rc = RefCounted<Type_>;

	/**
	 * Intrusive Reference Counted < Type_ >
	 */
	template <IsIntrusiveRefCountable Type_>
	using Rci = RefCountedIntrusive<Type_>;

	/**
	 * Atomic Reference Counted < Type_ >
	 */
	template <typename Type_>
	using Arc = smr<Type_>;

	/**
	 * Atomic Intrusive Reference Counted < Type_ >
	 */
	template <IncompleteOrAtomicIntrusiveRefCountable Type_, class Policy_ = StdAtomicPolicy>
	using Arci = AtomicRefCountedIntrusive<Type_, Policy_>;
}
