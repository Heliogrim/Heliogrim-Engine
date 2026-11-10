#pragma once
#include <Engine.Resource/Loader/Transformer.hpp>

#include "Traits.hpp"

namespace hg::editor::gfx::loader {
	class macro_novtable EditorTextureSubTransformer :
		public ::hg::engine::resource::loader::Transformer<engine::assets::TextureAsset, engine::gfx::TextureResource> {
	public:
		using this_type = EditorTextureSubTransformer;
		using underlying_type = ::hg::engine::resource::loader::Transformer<
			engine::assets::TextureAsset,
			engine::gfx::TextureResource
		>;

		using underlying_type::loader_traits;
		using underlying_type::request_type;
		using underlying_type::response_type;

		using underlying_type::stream_request_type;
		using underlying_type::stream_response_type;

	protected:
		EditorTextureSubTransformer() noexcept;

	public:
		~EditorTextureSubTransformer() noexcept override;

	public:
		[[nodiscard]] virtual bool canUse(nmpt<const engine::assets::TextureAsset> asset_) const noexcept = 0;
	};
}
