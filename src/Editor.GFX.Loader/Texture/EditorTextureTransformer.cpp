#include "EditorTextureTransformer.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Assets.System/IAssetRegistry.hpp>
#include <Engine.Assets/Assets.hpp>
#include <Engine.Assets/Types/Image.hpp>
#include <Engine.Core/Engine.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Heliogrim/ImageAsset.hpp>

using namespace ::hg::editor::gfx::loader;
using namespace ::hg;

/**/

[[nodiscard]] uptr<EditorTextureSubTransformer> build_proxy_transformer(
	const ptr<engine::gfx::loader::TextureTransformer> subject_
);

/**/

EditorTextureTransformer::EditorTextureTransformer(ref<engine::gfx::pool::GlobalResourcePool> pool_) :
	_pool(std::addressof(pool_)),
	_mainTransformer(std::addressof(pool_)),
	_transformers() {
	/**/
	_transformers.emplace_back(build_proxy_transformer(std::addressof(_mainTransformer)));
}

EditorTextureTransformer::EditorTextureTransformer(mref<EditorTextureTransformer> other_) noexcept :
	Transformer(std::move(other_)),
	_pool(std::exchange(other_._pool, nullptr)),
	_mainTransformer(std::move(other_._mainTransformer)),
	_transformers(std::move(other_._transformers)) {
	/**/
	if (not _transformers.empty()) {
		_transformers.front() = build_proxy_transformer(std::addressof(_mainTransformer));
	}
}

EditorTextureTransformer::~EditorTextureTransformer() = default;

nmpt<const EditorTextureSubTransformer> EditorTextureTransformer::select(
	nmpt<const engine::assets::TextureAsset> asset_
) const {

	auto iter = _transformers.rbegin();
	const auto end = _transformers.rend();
	for (; iter != end; ++iter) {
		if ((iter->get()->canUse(asset_))) {
			return iter->get();
		}
	}

	return nullptr;
}

ref<EditorTextureTransformer::this_type> EditorTextureTransformer::put(
	mref<uptr<EditorTextureSubTransformer>> transformer_
) & {
	_transformers.emplace_back(std::move(transformer_));
	return *this;
}

void EditorTextureTransformer::put(mref<uptr<EditorTextureSubTransformer>> transformer_) && {
	_transformers.emplace_back(std::move(transformer_));
}

void EditorTextureTransformer::drop(...) {
	::hg::todo_panic();
}

EditorTextureTransformer::response_type::type EditorTextureTransformer::operator()(
	mref<loader_traits::request::type> request_,
	mref<loader_traits::request::options> options_,
	cref<next_type> next_
) const {

	const auto selected = select(request_);
	if (selected == nullptr) {
		::hg::panic();
	}

	/**/

	return (*selected)(std::move(request_), std::move(options_), next_);
}

EditorTextureTransformer::stream_response_type::type EditorTextureTransformer::operator()(
	mref<loader_traits::stream_request::type> request_,
	mref<loader_traits::stream_request::options> options_,
	cref<next_type> next_
) const {

	const auto* const asset = Cast<engine::assets::TextureAsset>(request_->getAssociation());
	if (asset == nullptr) {
		::hg::panic();
	}

	/**/

	const auto selected = select(asset);
	if (selected == nullptr) {
		::hg::panic();
	}

	/**/

	return (*selected)(std::move(request_), std::move(options_), next_);
}

/**/

class EditorTextureMainTransformer final :
	public EditorTextureSubTransformer {
public:
	EditorTextureMainTransformer(const ptr<engine::gfx::loader::TextureTransformer> proxy_) noexcept :
		_proxy(proxy_) {}

	~EditorTextureMainTransformer() noexcept override = default;

private:
	nmpt<engine::gfx::loader::TextureTransformer> _proxy;

public:
	[[nodiscard]] bool canUse(nmpt<const engine::assets::TextureAsset> asset_) const noexcept override {

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
			if (source.path().name().ends_with(".ktx")) {
				return true;
			}
			if (source.path().name().ends_with(".ktx2")) {
				return true;
			}
		}

		/**/

		return false;
	}

public:
	[[nodiscard]] loader_traits::response::type operator()(
		mref<loader_traits::request::type> request_,
		mref<loader_traits::request::options> options_,
		cref<next_type> next_
	) const override {
		return (*_proxy)(std::move(request_), std::move(options_), next_);
	}

	loader_traits::stream_response::type operator()(
		mref<loader_traits::stream_request::type> request_,
		mref<loader_traits::stream_request::options> options_,
		cref<next_type> next_
	) const override {
		return (*_proxy)(std::move(request_), std::move(options_), next_);
	}
};

uptr<EditorTextureSubTransformer> build_proxy_transformer(
	const ptr<engine::gfx::loader::TextureTransformer> subject_
) {
	return make_uptr<EditorTextureMainTransformer>(subject_);
}
