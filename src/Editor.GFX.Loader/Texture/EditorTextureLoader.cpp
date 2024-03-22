#include "EditorTextureLoader.hpp"

using namespace ::hg::editor::gfx::loader;
using namespace ::hg;

EditorTextureLoader::EditorTextureLoader(
	nmpt<engine::gfx::cache::GlobalCacheCtrl> cache_,
	nmpt<engine::gfx::pool::GlobalResourcePool> pool_,
	mref<sptr<TextureSourceLoader>> sourceLoader_
) :
	EditorTextureLoaderType(
		EditorTextureCache(cache_),
		EditorTextureFeedback(),
		EditorTextureTransformer(pool_),
		std::move(sourceLoader_)
	) {}
