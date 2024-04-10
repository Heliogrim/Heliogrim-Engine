#include "Bmp.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets/Types/Image.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Reflect/Cast.hpp>

using namespace ::hg::editor::gfx::loader;
using namespace ::hg;

EditorBmpTextureTransformer::EditorBmpTextureTransformer() noexcept = default;

EditorBmpTextureTransformer::~EditorBmpTextureTransformer() noexcept = default;

bool EditorBmpTextureTransformer::canUse(nmpt<const engine::assets::TextureAsset> asset_) const noexcept {

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

		// /^(.+)\.(bmp)$/gi
		if (source.path().name().ends_with(".bmp")) {
			return true;
		}
	}

	/**/

	return false;
}

EditorBmpTextureTransformer::loader_traits::response::type EditorBmpTextureTransformer::operator()(
	mref<loader_traits::request::type> request_,
	mref<loader_traits::request::options> options_,
	cref<next_type> next_
) const {
	::hg::todo_panic();
}

EditorBmpTextureTransformer::loader_traits::stream_response::type EditorBmpTextureTransformer::operator()(
	mref<loader_traits::stream_request::type> request_,
	mref<loader_traits::stream_request::options> options_,
	cref<next_type> next_
) const {
	::hg::todo_panic();
}
