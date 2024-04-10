#include "Png.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets/Types/Image.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Reflect/Cast.hpp>

using namespace ::hg::editor::gfx::loader;
using namespace ::hg;

EditorPngTextureTransformer::EditorPngTextureTransformer() noexcept = default;

EditorPngTextureTransformer::~EditorPngTextureTransformer() noexcept = default;

bool EditorPngTextureTransformer::canUse(nmpt<const engine::assets::TextureAsset> asset_) const noexcept {

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
		if (source.path().name().ends_with(".png")) {
			return true;
		}
	}

	/**/

	return false;
}

EditorPngTextureTransformer::loader_traits::response::type EditorPngTextureTransformer::operator()(
	mref<loader_traits::request::type> request_,
	mref<loader_traits::request::options> options_,
	cref<next_type> next_
) const {
	::hg::todo_panic();
}

EditorPngTextureTransformer::loader_traits::stream_response::type EditorPngTextureTransformer::operator()(
	mref<loader_traits::stream_request::type> request_,
	mref<loader_traits::stream_request::options> options_,
	cref<next_type> next_
) const {
	::hg::todo_panic();
}
