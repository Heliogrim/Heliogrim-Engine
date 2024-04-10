#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.GFX/Cache/__fwd.hpp>
#include <Engine.GFX/Pool/__fwd.hpp>
#include <Engine.GFX/Graphics.hpp>
#include <Engine.Resource/ImporterManager.hpp>
#include <Engine.Resource/LoaderManager.hpp>

namespace hg::engine::gfx::loader {
	extern void register_loader(
		_In_ ref<resource::LoaderManager> manager_,
		_In_ ref<Graphics> graphics_,
		_In_ ref<gfx::cache::GlobalCacheCtrl> cacheCtrl_,
		_In_ ref<gfx::pool::GlobalResourcePool> pool_
	);

	extern void unregister_loader(
		_In_ ref<resource::LoaderManager> manager_,
		_In_ ref<Graphics> graphics_,
		_In_ ref<gfx::cache::GlobalCacheCtrl> cacheCtrl_,
		_In_ ref<gfx::pool::GlobalResourcePool> pool_
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
