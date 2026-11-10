#pragma once
#include "../EditorTextureSubTransformer.hpp"

namespace hg::engine::gfx::pool {
	class GlobalResourcePool;
}

namespace hg::editor::gfx::loader {
	class EditorBmpTextureTransformer final :
		public EditorTextureSubTransformer {
	public:
		using this_type = EditorBmpTextureTransformer;

	public:
		EditorBmpTextureTransformer(ref<engine::gfx::pool::GlobalResourcePool> pool_) noexcept;

		~EditorBmpTextureTransformer() noexcept override;

	private:
		nmpt<engine::gfx::pool::GlobalResourcePool> _pool;

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
