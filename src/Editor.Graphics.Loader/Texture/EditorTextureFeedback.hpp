#pragma once
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <Engine.Resource/Loader/Feedback.hpp>

#include "Traits.hpp"

namespace hg::editor::gfx::loader {
	class EditorTextureFeedback final :
		public ::hg::engine::resource::loader::Feedback<engine::assets::TextureAsset, engine::gfx::TextureResource> {
	public:
		using this_type = EditorTextureFeedback;
		using underlying_type = ::hg::engine::resource::loader::Feedback<
			engine::assets::TextureAsset,
			engine::gfx::TextureResource
		>;

		using underlying_type::loader_traits;
		using underlying_type::request_type;
		using underlying_type::response_type;

		using underlying_type::stream_request_type;
		using underlying_type::stream_response_type;

	public:
		EditorTextureFeedback();

		~EditorTextureFeedback() override;

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
