#pragma once

#include <Engine.Resource/Loader/CacheTraits.hpp>
#include <Engine.Resource/Loader/FeedbackTraits.hpp>
#include <Engine.Resource/Loader/LoaderTraits.hpp>
#include <Engine.Resource/Loader/SourceLoaderTraits.hpp>
#include <Engine.Resource/Loader/TransformerTraits.hpp>

#include "__fwd.hpp"

#include "FontLoadOptions.hpp"
#include "FontResource.hpp"

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::gfx::loader;

	template <>
	struct RequestOptions<LoaderRequest<assets::FontAsset>> {
		using type = FontLoadOptions;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::gfx::loader;

	template <>
	struct RequestOptions<CacheRequest<assets::FontAsset>> {
		using type = FontLoadOptions;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::gfx::loader;

	template <>
	struct RequestOptions<FeedbackRequest<assets::FontAsset>> {
		using type = FontLoadOptions;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::gfx::loader;

	template <>
	struct RequestOptions<SourceLoaderRequest<assets::FontAsset>> {
		using type = RequestOptions<SourceLoaderRequest<void>>::type;
	};

	template <>
	struct StreamOptions<SourceLoaderStreamRequest<assets::FontAsset>> {
		using type = StreamOptions<SourceLoaderStreamRequest<void>>::type;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::gfx::loader;

	template <>
	struct RequestOptions<TransformerRequest<assets::FontAsset>> {
		using type = FontLoadOptions;
	};
}
