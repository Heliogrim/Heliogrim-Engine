#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Graphics.Loader/Texture/TextureTransformer.hpp>
#include <Engine.Resource/Loader/Transformer.hpp>

#include "EditorTextureSubTransformer.hpp"
#include "Traits.hpp"

namespace hg::editor::graphics::loader {
	class EditorTextureTransformer final :
		public ::hg::engine::resource::loader::Transformer<engine::assets::TextureAsset, engine::graphics::TextureResource> {
	public:
		using this_type = EditorTextureTransformer;
		using underlying_type = ::hg::engine::resource::loader::Transformer<
			engine::assets::TextureAsset,
			engine::graphics::TextureResource
		>;

		using underlying_type::loader_traits;
		using underlying_type::request_type;
		using underlying_type::response_type;

		using underlying_type::stream_request_type;
		using underlying_type::stream_response_type;

	public:
		EditorTextureTransformer(ref<engine::graphics::pool::GlobalResourcePool> pool_);

		EditorTextureTransformer(const EditorTextureTransformer&) = delete;

		EditorTextureTransformer(mref<EditorTextureTransformer> other_) noexcept;

		~EditorTextureTransformer() override;

	private:
		nmpt<engine::graphics::pool::GlobalResourcePool> _pool;

		::hg::engine::graphics::loader::TextureTransformer _mainTransformer;
		Vector<uptr<EditorTextureSubTransformer>> _transformers;

	private:
		[[nodiscard]] _Success_(return != nullptr) nmpt<const EditorTextureSubTransformer> select(
			nmpt<const engine::assets::TextureAsset> asset_
		) const;

	public:
		ref<this_type> put(_In_ mref<uptr<EditorTextureSubTransformer>> transformer_) &;

		void put(_In_ mref<uptr<EditorTextureSubTransformer>> transformer_) &&;

		void drop(...);

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
