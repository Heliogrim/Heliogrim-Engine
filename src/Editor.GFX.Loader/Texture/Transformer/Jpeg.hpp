#pragma once
#include "../EditorTextureSubTransformer.hpp"

namespace hg::editor::gfx::loader {
	class EditorJpegTextureTransformer final :
		public EditorTextureSubTransformer {
	public:
		using this_type = EditorJpegTextureTransformer;

	public:
		EditorJpegTextureTransformer() noexcept;

		~EditorJpegTextureTransformer() noexcept override;

	public:
		[[nodiscard]] bool canUse(nmpt<const engine::assets::TextureAsset> asset_) const noexcept override;

	public:
		[[nodiscard]] loader_traits::response::type operator()(
			mref<loader_traits::request::type> request_,
			mref<loader_traits::request::options> options_,
			cref<next_type> next_
		) const override;

		loader_traits::stream_response::type operator()(
			mref<loader_traits::stream_request::type> request_,
			mref<loader_traits::stream_request::options> options_,
			cref<next_type> next_
		) const override;
	};
}
