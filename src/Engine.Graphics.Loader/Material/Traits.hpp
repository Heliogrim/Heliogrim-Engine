#pragma once

#include <Engine.Resource/Loader/CacheTraits.hpp>
#include <Engine.Resource/Loader/FeedbackTraits.hpp>
#include <Engine.Resource/Loader/LoaderTraits.hpp>
#include <Engine.Resource/Loader/SourceLoaderTraits.hpp>
#include <Engine.Resource/Loader/TransformerTraits.hpp>

#include "__fwd.hpp"

#include "MaterialLoadOptions.hpp"
#include "MaterialPrototypeLoadOptions.hpp"
#include "MaterialResource.hpp"
#include "MaterialPrototypeResource.hpp"

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
