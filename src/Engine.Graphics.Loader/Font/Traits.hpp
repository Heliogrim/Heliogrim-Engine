#pragma once

#include "__fwd.hpp"
#include "FontLoadOptions.hpp"
#include "FontResource.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Resource;
#endif

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::graphics::loader;

	template <>
	struct RequestOptions<LoaderRequest<assets::FontAsset>> {
		using type = FontLoadOptions;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::graphics::loader;

	template <>
	struct RequestOptions<CacheRequest<assets::FontAsset>> {
		using type = FontLoadOptions;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::graphics::loader;

	template <>
	struct RequestOptions<FeedbackRequest<assets::FontAsset>> {
		using type = FontLoadOptions;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::graphics::loader;

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
	using namespace ::hg::engine::graphics::loader;

	template <>
	struct RequestOptions<TransformerRequest<assets::FontAsset>> {
		using type = FontLoadOptions;
	};
}
