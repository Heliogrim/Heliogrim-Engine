#include "Jpeg.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets/Types/Image.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Reflect/Cast.hpp>

using namespace ::hg::editor::gfx::loader;
using namespace ::hg;

EditorJpegTextureTransformer::EditorJpegTextureTransformer() noexcept = default;

EditorJpegTextureTransformer::~EditorJpegTextureTransformer() noexcept = default;

bool EditorJpegTextureTransformer::canUse(nmpt<const engine::assets::TextureAsset> asset_) const noexcept {

	const auto* const registry = engine::Engine::getEngine()->getAssets()->getRegistry();
	const auto* const asset = registry->findAssetByGuid(asset_->baseImage());

	if (asset == nullptr) {
		return false;
	}

	/**/

	const auto* const image = Cast<engine::assets::Image>(asset);
	if (image == nullptr) {
		return false;
	}

	/**/

	for (const auto& source : image->sources()) {

		// /^(.+)\.(jp(e){0,1}g)$/gi
		if (source.path().name().ends_with(".jpg")) {
			return true;
		}

		if (source.path().name().ends_with(".jpeg")) {
			return true;
		}
	}

	/**/

	return false;
}

EditorJpegTextureTransformer::loader_traits::response::type EditorJpegTextureTransformer::operator()(
	mref<loader_traits::request::type> request_,
	mref<loader_traits::request::options> options_,
	cref<next_type> next_
) const {
	::hg::todo_panic();
}

EditorJpegTextureTransformer::loader_traits::stream_response::type EditorJpegTextureTransformer::operator()(
	mref<loader_traits::stream_request::type> request_,
	mref<loader_traits::stream_request::options> options_,
	cref<next_type> next_
) const {
	::hg::todo_panic();
}
