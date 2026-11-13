#pragma once

#include "Store/ResourceAccessStore.hpp"
#include "Store/ResourceObjectStore.hpp"
#include "Store/StackCache.hpp"

namespace hg::engine::io {
	class IoSystem;
}

namespace hg::engine::storage {
	struct Context {
		ResourceAccessStore accessStore;
		ResourceObjectStore objectStore;
		StackCache stacks;
		// Null or stopped: files are read and written synchronously on the calling thread.
		ptr<io::IoSystem> io = nullptr;
	};
}
