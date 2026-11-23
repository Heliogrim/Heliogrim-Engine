#pragma once

#include "__fwd.hpp"
#include "StaticGeometryLoadOptions.hpp"
#include "StaticGeometryResource.hpp"
#include "StaticGeometryStreamOptions.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Resource;
#endif

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::graphics::loader;

	template <>
	struct RequestOptions<LoaderRequest<assets::StaticGeometry>> {
		using type = StaticGeometryLoadOptions;
	};

	template <>
	struct StreamOptions<LoaderRequest<assets::StaticGeometry>> {
		using type = StaticGeometryStreamOptions;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::graphics::loader;

	template <>
	struct RequestOptions<CacheRequest<assets::StaticGeometry>> {
		using type = StaticGeometryLoadOptions;
	};

	template <>
	struct StreamOptions<CacheRequest<assets::StaticGeometry>> {
		using type = StaticGeometryStreamOptions;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::graphics::loader;

	template <>
	struct RequestOptions<FeedbackRequest<assets::StaticGeometry>> {
		using type = StaticGeometryLoadOptions;
	};

	template <>
	struct StreamOptions<FeedbackRequest<assets::StaticGeometry>> {
		using type = StaticGeometryStreamOptions;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::graphics::loader;

	template <>
	struct RequestOptions<SourceLoaderRequest<assets::StaticGeometry>> {
		using type = RequestOptions<SourceLoaderRequest<void>>::type;
	};

	template <>
	struct StreamOptions<SourceLoaderStreamRequest<assets::StaticGeometry>> {
		using type = StreamOptions<SourceLoaderStreamRequest<void>>::type;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::graphics::loader;

	template <>
	struct RequestOptions<TransformerRequest<assets::StaticGeometry>> {
		using type = StaticGeometryLoadOptions;
	};

	template <>
	struct StreamOptions<TransformerRequest<assets::StaticGeometry>> {
		using type = StaticGeometryStreamOptions;
	};
}
