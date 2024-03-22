#pragma once

#include <Engine.Resource/Loader/CacheTraits.hpp>
#include <Engine.Resource/Loader/FeedbackTraits.hpp>
#include <Engine.Resource/Loader/LoaderTraits.hpp>
#include <Engine.Resource/Loader/SourceLoaderTraits.hpp>
#include <Engine.Resource/Loader/TransformerTraits.hpp>

#include "__fwd.hpp"

#include "TextureLoadOptions.hpp"
#include "TextureResource.hpp"
#include "TextureStreamOptions.hpp"

/* Loader Interface */

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::gfx::loader;

	template <>
	struct resource::loader::RequestOptions<LoaderRequest<assets::TextureAsset>> {
		using type = TextureLoadOptions;
	};

	template <>
	struct resource::loader::StreamOptions<LoaderStreamRequest<assets::TextureAsset>> {
		using type = TextureStreamOptions;
	};
}

/* Cache Interface */

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::gfx::loader;

	template <>
	struct resource::loader::RequestOptions<CacheRequest<assets::TextureAsset>> {
		using type = TextureLoadOptions;
	};

	template <>
	struct resource::loader::StreamOptions<CacheStreamRequest<assets::TextureAsset>> {
		using type = TextureStreamOptions;
	};
}

/* Feedback Interface */

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::gfx::loader;

	template <>
	struct resource::loader::RequestOptions<FeedbackRequest<assets::TextureAsset>> {
		using type = TextureLoadOptions;
	};

	template <>
	struct resource::loader::StreamOptions<FeedbackStreamRequest<assets::TextureAsset>> {
		using type = TextureStreamOptions;
	};
}

/* Transformer Interface */

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::gfx::loader;

	template <>
	struct RequestOptions<TransformerRequest<assets::TextureAsset>> {
		using type = TextureLoadOptions;
	};

	template <>
	struct StreamOptions<TransformerStreamRequest<assets::TextureAsset>> {
		using type = TextureStreamOptions;
	};
}

/* Source Loader Interface */

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::gfx::loader;

	template <>
	struct RequestOptions<SourceLoaderRequest<assets::TextureAsset>> {
		using type = RequestOptions<SourceLoaderRequest<void>>::type;
	};

	template <>
	struct StreamOptions<SourceLoaderRequest<assets::TextureAsset>> {
		using type = StreamOptions<SourceLoaderStreamRequest<void>>::type;
	};
}
