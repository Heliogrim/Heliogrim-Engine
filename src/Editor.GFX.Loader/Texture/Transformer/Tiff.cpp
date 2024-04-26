#include "Tiff.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets/Types/Image.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Reflect/Cast.hpp>

using namespace ::hg::editor::gfx::loader;
using namespace ::hg;

EditorTiffTextureTransformer::EditorTiffTextureTransformer() noexcept = default;

EditorTiffTextureTransformer::~EditorTiffTextureTransformer() noexcept = default;

bool EditorTiffTextureTransformer::canUse(nmpt<const engine::assets::TextureAsset> asset_) const noexcept {

	const auto registry = engine::Engine::getEngine()->getAssets()->getRegistry();
	const auto asset = registry->findAssetByGuid(asset_->baseImage());

	if (asset == nullptr) {
		return false;
	}

	/**/

	const auto* const image = Cast<engine::assets::Image>(asset.get());
	if (image == nullptr) {
		return false;
	}

	/**/

	for (const auto& source : image->sources()) {

		// /^(.+)\.(ti(f){1,2})$/gi
		if (source.path().name().ends_with(".tif")) {
			return true;
		}

		if (source.path().name().ends_with(".tiff")) {
			return true;
		}
	}

	/**/

	return false;
}

EditorTiffTextureTransformer::loader_traits::response::type EditorTiffTextureTransformer::operator()(
	mref<loader_traits::request::type> request_,
	mref<loader_traits::request::options> options_,
	cref<next_type> next_
) const {
	::hg::todo_panic();
}

EditorTiffTextureTransformer::loader_traits::stream_response::type EditorTiffTextureTransformer::operator()(
	mref<loader_traits::stream_request::type> request_,
	mref<loader_traits::stream_request::options> options_,
	cref<next_type> next_
) const {
	::hg::todo_panic();
}
