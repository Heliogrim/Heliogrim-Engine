#pragma once
#include "../Concurrent/SharedMemoryReference.hpp"

#include "RefCounted.hpp"
#include "RefCountedIntrusive.hpp"
#include "AtomicRefCounted.hpp"
#include "AtomicRefCountedIntrusive.hpp"

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
	template <IncompleteOrAtomicIntrusiveRefCountable Type_>
	using Arci = AtomicRefCountedIntrusive<Type_>;
}
