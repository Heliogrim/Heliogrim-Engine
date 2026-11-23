#pragma once

#include "__fwd.hpp"
#include "EffectLoadOptions.hpp"
#include "EffectResource.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Resource;
#endif

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::accel::loader;

	template <>
	struct RequestOptions<LoaderRequest<assets::AccelEffect>> {
		using type = EffectLoadOptions;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::accel::loader;

	template <>
	struct RequestOptions<CacheRequest<assets::AccelEffect>> {
		using type = EffectLoadOptions;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::accel::loader;

	template <>
	struct RequestOptions<FeedbackRequest<assets::AccelEffect>> {
		using type = EffectLoadOptions;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::accel::loader;

	template <>
	struct RequestOptions<SourceLoaderRequest<assets::AccelEffect>> {
		using type = RequestOptions<SourceLoaderRequest<void>>::type;
	};
}

namespace hg::engine::resource::loader {
	using namespace ::hg::engine::accel::loader;

	template <>
	struct RequestOptions<TransformerRequest<assets::AccelEffect>> {
		using type = EffectLoadOptions;
	};
}
