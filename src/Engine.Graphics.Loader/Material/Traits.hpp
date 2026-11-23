#pragma once

#include "__fwd.hpp"
#include "MaterialLoadOptions.hpp"
#include "MaterialPrototypeLoadOptions.hpp"
#include "MaterialPrototypeResource.hpp"
#include "MaterialResource.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Resource;
#endif

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::graphics::loader;

	template <>
	struct RequestOptions<LoaderRequest<assets::GraphicsMaterial>> {
		using type = MaterialLoadOptions;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::graphics::loader;

	template <>
	struct RequestOptions<CacheRequest<assets::GraphicsMaterial>> {
		using type = MaterialLoadOptions;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::graphics::loader;

	template <>
	struct RequestOptions<FeedbackRequest<assets::GraphicsMaterial>> {
		using type = MaterialLoadOptions;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::graphics::loader;

	template <>
	struct RequestOptions<SourceLoaderRequest<assets::GraphicsMaterial>> {
		using type = RequestOptions<SourceLoaderRequest<void>>::type;
	};

	template <>
	struct StreamOptions<SourceLoaderStreamRequest<assets::GraphicsMaterial>> {
		using type = StreamOptions<SourceLoaderStreamRequest<void>>::type;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::graphics::loader;

	template <>
	struct RequestOptions<TransformerRequest<assets::GraphicsMaterial>> {
		using type = MaterialLoadOptions;
	};
}

/* Material Prototype */

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::graphics::loader;

	template <>
	struct RequestOptions<LoaderRequest<assets::GraphicsMaterialPrototype>> {
		using type = MaterialPrototypeLoadOptions;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::graphics::loader;

	template <>
	struct RequestOptions<CacheRequest<assets::GraphicsMaterialPrototype>> {
		using type = MaterialPrototypeLoadOptions;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::graphics::loader;

	template <>
	struct RequestOptions<FeedbackRequest<assets::GraphicsMaterialPrototype>> {
		using type = MaterialPrototypeLoadOptions;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::graphics::loader;

	template <>
	struct RequestOptions<SourceLoaderRequest<assets::GraphicsMaterialPrototype>> {
		using type = RequestOptions<SourceLoaderRequest<void>>::type;
	};

	template <>
	struct StreamOptions<SourceLoaderStreamRequest<assets::GraphicsMaterialPrototype>> {
		using type = StreamOptions<SourceLoaderStreamRequest<void>>::type;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::graphics::loader;

	template <>
	struct RequestOptions<TransformerRequest<assets::GraphicsMaterialPrototype>> {
		using type = MaterialPrototypeLoadOptions;
	};
}
