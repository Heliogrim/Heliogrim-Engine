#include "EditorTextureLoader.hpp"

using namespace ::hg::editor::gfx::loader;
using namespace ::hg;

EditorTextureLoader::EditorTextureLoader(
	ref<engine::gfx::cache::GlobalCacheCtrl> cache_,
	ref<engine::gfx::pool::GlobalResourcePool> pool_,
	mref<sptr<TextureSourceLoader>> sourceLoader_
) :
	EditorTextureLoaderType(
		EditorTextureCache(cache_),
		EditorTextureFeedback(),
		EditorTextureTransformer(pool_),
		std::move(sourceLoader_)
	) {}
