#pragma once

#include "Store/ResourceCache.hpp"
#include "Store/ResourceStore.hpp"

namespace hg::engine::storage {
	struct Context {
		ResourceStore store;
		ResourceCache caches;
	};
}
