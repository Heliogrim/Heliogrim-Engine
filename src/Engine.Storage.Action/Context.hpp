#pragma once

#include "Store/ResourceAccessStore.hpp"
#include "Store/ResourceObjectStore.hpp"
#include "Store/StackCache.hpp"

namespace hg::engine::storage {
	struct Context {
		ResourceAccessStore accessStore;
		ResourceObjectStore objectStore;
		StackCache stacks;
	};
}
