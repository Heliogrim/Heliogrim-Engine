#include "EditorTextureLoader.hpp"

using namespace ::hg::editor::graphics::loader;
using namespace ::hg;

EditorTextureLoader::EditorTextureLoader(
	ref<engine::graphics::cache::GlobalCacheCtrl> cache_,
	ref<engine::graphics::pool::GlobalResourcePool> pool_,
	mref<sptr<TextureSourceLoader>> sourceLoader_
) :
	EditorTextureLoaderType(
		EditorTextureCache(cache_),
		EditorTextureFeedback(),
		EditorTextureTransformer(pool_),
		std::move(sourceLoader_)
	) {}
