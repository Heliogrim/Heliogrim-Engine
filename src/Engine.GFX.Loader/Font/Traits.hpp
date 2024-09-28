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
	struct RequestOptions<LoaderRequest<assets::Font>> {
		using type = FontLoadOptions;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::gfx::loader;

	template <>
	struct RequestOptions<CacheRequest<assets::Font>> {
		using type = FontLoadOptions;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::gfx::loader;

	template <>
	struct RequestOptions<FeedbackRequest<assets::Font>> {
		using type = FontLoadOptions;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::gfx::loader;

	template <>
	struct RequestOptions<SourceLoaderRequest<assets::Font>> {
		using type = RequestOptions<SourceLoaderRequest<void>>::type;
	};

	template <>
	struct StreamOptions<SourceLoaderStreamRequest<assets::Font>> {
		using type = StreamOptions<SourceLoaderStreamRequest<void>>::type;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::gfx::loader;

	template <>
	struct RequestOptions<TransformerRequest<assets::Font>> {
		using type = FontLoadOptions;
	};
}
