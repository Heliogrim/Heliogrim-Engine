#pragma once

#include <Engine.Pedantic.Resource/Resource.hpp>
#include <Engine.Resource.Blob/Blob.hpp>
#include <Engine.Storage.Registry/IStorage.hpp>

#include "__fwd.hpp"

#include "SourceLoader/SourceLoadOptions.hpp"
#include "SourceLoader/SourceStreamOptions.hpp"

namespace hg::engine::storage {
	class StorageSystem;
}

namespace hg::engine::resource::loader {
	class SourceLoader;

	/**/

	template <>
	struct RequestOptions<SourceLoaderRequest<void>> {
		using this_type = RequestOptions<SourceLoaderRequest<void>>;
		using type = SourceLoadOptions;
	};

	template <typename AssetType_>
	struct RequestOptions<SourceLoaderRequest<AssetType_>> {
		using type = RequestOptions<SourceLoaderRequest<void>>::type;
	};

	template <>
	struct StreamOptions<SourceLoaderStreamRequest<void>> {
		using this_type = StreamOptions<SourceLoaderStreamRequest<void>>;
		using type = SourceStreamOptions;
	};

	template <typename AssetType_>
	struct StreamOptions<SourceLoaderStreamRequest<AssetType_>> {
		using type = StreamOptions<SourceLoaderStreamRequest<void>>::type;
	};

	/**/

	template <typename Ty_>
	struct SourceLoaderRequest {
		using this_type = SourceLoaderRequest<void>;

		template <typename Type_>
		using wrapper_type = non_owning_rptr<Type_>;
		using value_type = const assets::Asset;

		using type = wrapper_type<value_type>;

		/**/

		template <typename Type_>
		using options_wrapper_type = std::type_identity_t<Type_>;
		using options_type = typename RequestOptions<this_type>::type;

		using options = options_wrapper_type<options_type>;
	};

	template <typename Ty_>
	struct SourceLoaderResponse {
		using this_type = SourceLoaderResponse<void>;

		template <typename Type_>
		using wrapper_type = std::type_identity_t<Type_>;
		//using value_type = storage::AccessBlobReadonly;
		using value_type = std::pair<ref<storage::StorageSystem>, Arci<storage::IStorage>>;

		using type = wrapper_type<value_type>;
	};

	/**/

	template <typename Ty_>
	struct SourceLoaderStreamRequest {
		using this_type = SourceLoaderStreamRequest<void>;

		template <typename Type_>
		using wrapper_type = smr<Type_>;
		using value_type = assets::Asset;

		using type = wrapper_type<value_type>;

		/**/

		template <typename Type_>
		using options_wrapper_type = std::type_identity_t<Type_>;
		using options_type = typename StreamOptions<this_type>::type;

		using options = options_wrapper_type<options_type>;
	};

	template <typename Ty_>
	struct SourceLoaderStreamResponse {
		using this_type = SourceLoaderStreamResponse<void>;

		template <typename Type_>
		using wrapper_type = std::type_identity_t<Type_>;
		//using value_type = storage::AccessBlobReadonly;
		using value_type = std::pair<ref<storage::StorageSystem>, Arci<storage::IStorage>>;

		using type = wrapper_type<value_type>;
	};
}
