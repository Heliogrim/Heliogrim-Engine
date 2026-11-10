#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Graphics/Graphics.hpp>
#include <Engine.Graphics/Cache/__fwd.hpp>
#include <Engine.Graphics/Pool/__fwd.hpp>
#include <Engine.Resource/ImporterManager.hpp>
#include <Engine.Resource/LoaderManager.hpp>

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
