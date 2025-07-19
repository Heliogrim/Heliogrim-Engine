#pragma once

#include <Engine.Common/Wrapper.hpp>

#include "__fwd.hpp"

namespace hg::engine::resource::loader {
	template <IsRequestValueType ValueType_>
	struct LoaderRequest {
		using this_type = LoaderRequest<ValueType_>;

		template <typename Type_>
		using wrapper_type = non_owning_rptr<Type_>;
		using value_type = ValueType_;

		using type = wrapper_type<const value_type>;

		/**/

		template <typename Type_>
		using options_wrapper_type = std::type_identity_t<Type_>;
		using options_type = typename RequestOptions<this_type>::type;

		using options = options_wrapper_type<options_type>;
	};

	template <IsStreamRequestSubjectType SubjectType_>
	struct LoaderStreamRequest {
		using this_type = LoaderStreamRequest<SubjectType_>;

		template <typename Type_>
		using wrapper_type = smr<Type_>;
		using value_type = ResourceBase;

		using type = wrapper_type<const value_type>;

		/**/

		template <typename Type_>
		using options_wrapper_type = std::type_identity_t<Type_>;
		using options_type = typename StreamOptions<this_type>::type;

		using options = options_wrapper_type<options_type>;
	};

	template <IsResponseValueType ValueType_>
	struct LoaderResponse {
		using this_type = LoaderResponse<ValueType_>;

		template <typename Type_>
		using wrapper_type = smr<Type_>;
		using value_type = ValueType_;

		using type = wrapper_type<value_type>;
	};

	template <IsStreamResponseSubjectType SubjectType_>
	struct LoaderStreamResponse {
		using this_type = LoaderStreamResponse<SubjectType_>;

		template <typename Type_>
		using wrapper_type = std::type_identity_t<Type_>;
		using value_type = void;

		using type = wrapper_type<value_type>;
	};

	/**/

	template <typename RequestValueType_, bool Streamable_>
	struct __LoaderRequestSelector {
		using type = LoaderRequest<RequestValueType_>;
		using stream_type = void;
	};

	template <typename RequestValueType_>
	struct __LoaderRequestSelector<RequestValueType_, true> {
		using type = LoaderRequest<RequestValueType_>;
		using stream_type = LoaderStreamRequest<RequestValueType_>;
	};

	/**/

	template <typename RequestValueType_, typename ResponseValueType_, bool Streamable_>
	struct __LoaderResponseSelector {
		using type = LoaderResponse<ResponseValueType_>;
		using stream_type = void;
	};

	template <typename RequestValueType_, typename ResponseValueType_>
	struct __LoaderResponseSelector<RequestValueType_, ResponseValueType_, true> {
		using type = LoaderResponse<ResponseValueType_>;
		using stream_type = LoaderStreamResponse<RequestValueType_>;
	};

	/**/

	template <typename RequestValueType_, typename ResponseValueType_>
	struct LoaderTraits {
		using request = typename __LoaderRequestSelector<RequestValueType_,
			assets::IsStreamableAsset<RequestValueType_>>::type;
		using stream_request = typename __LoaderRequestSelector<RequestValueType_,
			assets::IsStreamableAsset<RequestValueType_>>::stream_type;

		using response = typename __LoaderResponseSelector<RequestValueType_, ResponseValueType_,
			assets::IsStreamableAsset<RequestValueType_>>::type;
		using stream_response = typename __LoaderResponseSelector<RequestValueType_, ResponseValueType_,
			assets::IsStreamableAsset<RequestValueType_>>::stream_type;
	};
}
