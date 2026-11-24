#pragma once

#include <Engine.Common/Sal.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Graphics;
import Engine.Graphics.Loader;
import Engine.Resource;
#endif

/* Local forward declaration: Engine.Graphics.Core's Graphics.cpp includes this header next to its textual Graphics.hpp. */
namespace hg::engine {
	class Graphics;
}

namespace hg::engine::graphics::loader {
	extern void register_loader(
		_In_ ref<resource::LoaderManager> manager_,
		_In_ ref<Graphics> graphics_,
		_In_ ref<graphics::cache::GlobalCacheCtrl> cacheCtrl_,
		_In_ ref<graphics::pool::GlobalResourcePool> pool_
	);

	extern void unregister_loader(
		_In_ ref<resource::LoaderManager> manager_,
		_In_ ref<Graphics> graphics_,
		_In_ ref<graphics::cache::GlobalCacheCtrl> cacheCtrl_,
		_In_ ref<graphics::pool::GlobalResourcePool> pool_
	);

	extern void register_importer(
		_In_ ref<res::ImporterManager> manager_,
		_In_ ref<Graphics> graphics_
	);

	extern void unregister_importer(
		_In_ ref<res::ImporterManager> manager_,
		_In_ ref<Graphics> graphics_
	);
}
