#include "EditorTextureTransformer.hpp"

#include <Engine.Reflect/Cast.hpp>

using namespace ::hg::editor::gfx::loader;
using namespace ::hg;

/**/

[[nodiscard]] uptr<EditorTextureSubTransformer> build_proxy_transformer(
	const ptr<engine::gfx::loader::TextureTransformer> subject_
);

/**/

EditorTextureTransformer::EditorTextureTransformer(nmpt<engine::gfx::pool::GlobalResourcePool> pool_) :
	_pool(pool_),
	_mainTransformer(pool_.get()),
	_transformers() {
	/**/
	_transformers.emplace_back(build_proxy_transformer(std::addressof(_mainTransformer)));
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
	EditorTextureMainTransformer(const ptr<engine::gfx::loader::TextureTransformer> proxy_) :
		_proxy(proxy_) {}

	~EditorTextureMainTransformer() override = default;

private:
	nmpt<engine::gfx::loader::TextureTransformer> _proxy;

public:
	[[nodiscard]] bool canUse(nmpt<const engine::assets::TextureAsset> asset_) const noexcept override {
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
