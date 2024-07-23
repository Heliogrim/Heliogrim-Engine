#pragma once

#include <Engine.Common/Sal.hpp>

#include "__fwd.hpp"

#include "LoaderTraits.hpp"

#include "Loader.hpp"
#include "Cache.hpp"
#include "Feedback.hpp"
#include "Transformer.hpp"
#include "SourceLoader/SourceLoader.hpp"

namespace hg::engine::resource::loader {
	template <
		template <typename> typename Type_,
		template <typename> typename... Rest_
	>
	struct MetaTypeWrapperList {
		using next = MetaTypeWrapperList<Rest_...>;

		template <typename Ty_>
		using type = Type_<Ty_>;

		constexpr size_t count() const noexcept {
			return next::count() + 1;
		}
	};

	template <template <typename> typename Type_>
	struct MetaTypeWrapperList<Type_> {
		using next = void;
		template <typename Ty_>
		using type = Type_<Ty_>;

		constexpr size_t count() const noexcept {
			return 1;
		}
	};

	/**/

	template <
		IsRequestValueType RequestType_,
		IsResponseValueType ResponseType_ = ResourceBase,
		typename StageRefTypes_ = MetaTypeWrapperList<
			std::type_identity_t,
			std::type_identity_t,
			std::type_identity_t,
			sptr
		>,
		class CacheStageType_ = Cache<RequestType_, ResponseType_>,
		class FeedbackStageType_ = Feedback<RequestType_, ResponseType_>,
		class TransformerStageType_ = Transformer<RequestType_, ResponseType_>,
		class SourceLoaderType_ = SourceLoader>
	class LoaderChainBase :
		public InheritMeta<LoaderChainBase<RequestType_, ResponseType_, StageRefTypes_, CacheStageType_,
			FeedbackStageType_, TransformerStageType_, SourceLoaderType_>, Loader<RequestType_, ResponseType_>> {
	public:
		template <
			IsRequestValueType,
			IsResponseValueType,
			typename,
			class,
			class,
			class,
			class,
			bool>
		friend class LoaderChain;

	public:
		using this_type = LoaderChainBase<
			RequestType_,
			ResponseType_,
			StageRefTypes_,
			CacheStageType_,
			FeedbackStageType_,
			TransformerStageType_,
			SourceLoaderType_
		>;
		using underlying_type = Loader<RequestType_, ResponseType_>;

		using traits = LoaderTraits<RequestType_, ResponseType_>;

		/**/

		using cache_ref = typename StageRefTypes_;
		template <typename Type_>
		using cache_ref_type = typename cache_ref::template type<Type_>;
		using cache_stage_type = CacheStageType_;

		using cache_type = cache_ref_type<cache_stage_type>;

		using feedback_ref = typename cache_ref::next;
		template <typename Type_>
		using feedback_ref_type = typename feedback_ref::template type<Type_>;
		using feedback_stage_type = FeedbackStageType_;

		using feedback_type = feedback_ref_type<feedback_stage_type>;

		using transformer_ref = typename feedback_ref::next;
		template <typename Type_>
		using transformer_ref_type = typename transformer_ref::template type<Type_>;
		using transformer_stage_type = TransformerStageType_;

		using transformer_type = transformer_ref_type<transformer_stage_type>;

		using source_loader_ref = typename transformer_ref::next;
		template <typename Type_>
		using source_loader_ref_type = typename source_loader_ref::template type<Type_>;
		using source_loader_stage_type = SourceLoaderType_;

		using source_loader_type = source_loader_ref_type<source_loader_stage_type>;

	public:
		cache_type cache;
		feedback_type feedback;
		transformer_type transformer;
		source_loader_type sourceLoader;

	public:
		template <
			typename CacheTx_,
			typename FeedbackTx_,
			typename TransformerTx_,
			typename SourceLoaderTx_
		>
		constexpr LoaderChainBase(
			CacheTx_&& cache_,
			FeedbackTx_&& feedback_,
			TransformerTx_&& transformer_,
			SourceLoaderTx_&& sourceLoader_
		) :
			InheritMeta<this_type, underlying_type>(),
			cache(std::forward<CacheTx_>(cache_)),
			feedback(std::forward<FeedbackTx_>(feedback_)),
			transformer(std::forward<TransformerTx_>(transformer_)),
			sourceLoader(std::forward<SourceLoaderTx_>(sourceLoader_)),
			transformerLink(sourceLoader),
			feedbackLink(transformer, transformerLink),
			cacheLink(feedback, feedbackLink) {}

	private:
		/* Stage Invoker Helper */

		template <typename Stage_, typename Result_, typename... Args_>
		[[nodiscard]] FORCE_INLINE static Result_ invoke_stage(cref<std::decay_t<Stage_>> stage_, Args_&&... args_) {
			return stage_(std::forward<Args_>(args_)...);
		}

		template <typename Stage_, typename Result_, typename... Args_>
		[[nodiscard]] FORCE_INLINE static Result_ invoke_stage(
			cref<uptr<std::decay_t<Stage_>>> stage_,
			Args_&&... args_
		) {
			return invoke_stage<Stage_, Result_, Args_...>(*stage_, std::forward<Args_>(args_)...);
		}

		template <typename Stage_, typename Result_, typename... Args_>
		[[nodiscard]] FORCE_INLINE static Result_ invoke_stage(
			cref<sptr<std::decay_t<Stage_>>> stage_,
			Args_&&... args_
		) {
			return invoke_stage<Stage_, Result_, Args_...>(*stage_, std::forward<Args_>(args_)...);
		}

		template <typename Stage_, typename Result_, typename... Args_>
		[[nodiscard]] FORCE_INLINE static Result_ invoke_stage(
			cref<smr<std::decay_t<Stage_>>> stage_,
			Args_&&... args_
		) {
			return invoke_stage<Stage_, Result_, Args_...>(*stage_, std::forward<Args_>(args_)...);
		}

		/* Chain Link Implementations */

		template <typename FwdType_, bool Streamable_>
		struct TransformerChainLink :
			public TransformerNextLink<RequestType_, ResponseType_, Streamable_> {
			using base_type = TransformerNextLink<RequestType_, ResponseType_, Streamable_>;

			using next_response_type = typename base_type::next_response_type;
			using next_request_type = typename base_type::next_request_type;

			using next_stream_request_type = typename base_type::next_stream_request_type;
			using next_stream_response_type = typename base_type::next_stream_response_type;

		public:
			constexpr TransformerChainLink(cref<source_loader_type> stage_) noexcept :
				stage(stage_) {}

			~TransformerChainLink() override = default;

		public:
			cref<source_loader_type> stage;

		public:
			[[nodiscard]] typename next_response_type::type operator()(
				_In_ mref<typename next_request_type::type> request_,
				_In_ mref<typename next_request_type::options> options_
			) const noexcept override {
				return invoke_stage<source_loader_stage_type, typename next_response_type::type>(
					stage,
					std::move(request_),
					std::move(options_)
				);
			}
		};

		template <typename RequestType_>
		struct TransformerChainLink<RequestType_, true> :
			public TransformerNextLink<RequestType_, ResponseType_, true> {
			using base_type = TransformerNextLink<RequestType_, ResponseType_, true>;

			using next_response_type = typename base_type::next_response_type;
			using next_request_type = typename base_type::next_request_type;

			using next_stream_request_type = typename base_type::next_stream_request_type;
			using next_stream_response_type = typename base_type::next_stream_response_type;

		public:
			constexpr TransformerChainLink(cref<source_loader_type> stage_) noexcept :
				stage(stage_) {}

			~TransformerChainLink() override = default;

		public:
			cref<source_loader_type> stage;

		public:
			[[nodiscard]] typename next_response_type::type operator()(
				_In_ mref<typename next_request_type::type> request_,
				_In_ mref<typename next_request_type::options> options_
			) const noexcept override {
				return invoke_stage<source_loader_stage_type, typename next_response_type::type>(
					stage,
					std::move(request_),
					std::move(options_)
				);
			}

			[[nodiscard]] typename next_stream_response_type::type operator()(
				_In_ mref<typename next_stream_request_type::type> request_,
				_In_ mref<typename next_stream_request_type::options> options_
			) const noexcept override {
				return invoke_stage<source_loader_stage_type, typename next_response_type::type>(
					stage,
					std::move(request_),
					std::move(options_)
				);
			}
		};

		/**/

		template <typename FwdType_, bool Streamable_>
		struct FeedbackChainLink :
			public FeedbackNextLink<RequestType_, ResponseType_, Streamable_> {
			using base_type = FeedbackNextLink<RequestType_, ResponseType_, Streamable_>;

			using next_response_type = typename base_type::next_response_type;
			using next_request_type = typename base_type::next_request_type;

			using next_stream_request_type = typename base_type::next_stream_request_type;
			using next_stream_response_type = typename base_type::next_stream_response_type;

		public:
			constexpr FeedbackChainLink(
				cref<transformer_type> stage_,
				cref<TransformerChainLink<FwdType_, Streamable_>> nextLink_
			) noexcept :
				stage(stage_),
				nextLink(nextLink_) {}

			~FeedbackChainLink() override = default;

		public:
			cref<transformer_type> stage;
			cref<TransformerChainLink<FwdType_, Streamable_>> nextLink;

		public:
			[[nodiscard]] typename next_response_type::type operator()(
				_In_ mref<typename next_request_type::type> request_,
				_In_ mref<typename next_request_type::options> options_
			) const noexcept override {
				return invoke_stage<transformer_stage_type, typename next_response_type::type>(
					stage,
					std::move(request_),
					std::move(options_),
					nextLink
				);
			}
		};

		template <typename FwdType_>
		struct FeedbackChainLink<FwdType_, true> :
			public FeedbackNextLink<RequestType_, ResponseType_, true> {
			using base_type = FeedbackNextLink<RequestType_, ResponseType_, true>;

			using next_response_type = typename base_type::next_response_type;
			using next_request_type = typename base_type::next_request_type;

			using next_stream_request_type = typename base_type::next_stream_request_type;
			using next_stream_response_type = typename base_type::next_stream_response_type;

		public:
			constexpr FeedbackChainLink(
				cref<transformer_type> stage_,
				cref<TransformerChainLink<FwdType_, true>> nextLink_
			) noexcept :
				stage(stage_),
				nextLink(nextLink_) {}

			~FeedbackChainLink() override = default;

		public:
			cref<transformer_type> stage;
			cref<TransformerChainLink<FwdType_, true>> nextLink;

		public:
			[[nodiscard]] typename next_response_type::type operator()(
				_In_ mref<typename next_request_type::type> request_,
				_In_ mref<typename next_request_type::options> options_
			) const noexcept override {
				return invoke_stage<transformer_stage_type, typename next_response_type::type>(
					stage,
					std::move(request_),
					std::move(options_),
					nextLink
				);
			}

			[[nodiscard]] typename next_stream_response_type::type operator()(
				_In_ mref<typename next_stream_request_type::type> request_,
				_In_ mref<typename next_stream_request_type::options> options_
			) const noexcept override {
				return invoke_stage<transformer_stage_type, typename next_stream_response_type::type>(
					stage,
					std::move(request_),
					std::move(options_),
					nextLink
				);
			}
		};

		/**/

		template <typename FwdType_, bool Streamable_>
		struct CacheChainLink :
			public CacheNextLink<RequestType_, ResponseType_, Streamable_> {
			using base_type = CacheNextLink<RequestType_, ResponseType_, Streamable_>;

			using next_response_type = typename base_type::next_response_type;
			using next_request_type = typename base_type::next_request_type;

			using next_stream_request_type = typename base_type::next_stream_request_type;
			using next_stream_response_type = typename base_type::next_stream_response_type;

		public:
			constexpr CacheChainLink(
				cref<feedback_type> stage_,
				cref<FeedbackChainLink<FwdType_, Streamable_>> nextLink_
			) noexcept :
				stage(stage_),
				nextLink(nextLink_) {}

			~CacheChainLink() override = default;

		public:
			cref<feedback_type> stage;
			cref<FeedbackChainLink<FwdType_, Streamable_>> nextLink;

		public:
			[[nodiscard]] typename next_response_type::type operator()(
				_In_ mref<typename next_request_type::type> request_,
				_In_ mref<typename next_request_type::options> options_
			) const noexcept override {
				return invoke_stage<feedback_stage_type, typename next_response_type::type>(
					stage,
					std::move(request_),
					std::move(options_),
					nextLink
				);
			}
		};

		template <typename FwdType_>
		struct CacheChainLink<FwdType_, true> :
			public CacheNextLink<RequestType_, ResponseType_, true> {
			using base_type = CacheNextLink<RequestType_, ResponseType_, true>;

			using next_response_type = typename base_type::next_response_type;
			using next_request_type = typename base_type::next_request_type;

			using next_stream_request_type = typename base_type::next_stream_request_type;
			using next_stream_response_type = typename base_type::next_stream_response_type;

		public:
			constexpr CacheChainLink(
				cref<feedback_type> stage_,
				cref<FeedbackChainLink<FwdType_, true>> nextLink_
			) noexcept :
				stage(stage_),
				nextLink(nextLink_) {}

			~CacheChainLink() override = default;

		public:
			cref<feedback_type> stage;
			cref<FeedbackChainLink<FwdType_, true>> nextLink;

		public:
			[[nodiscard]] typename next_response_type::type operator()(
				_In_ mref<typename next_request_type::type> request_,
				_In_ mref<typename next_request_type::options> options_
			) const noexcept override {
				return invoke_stage<feedback_stage_type, typename next_response_type::type>(
					stage,
					std::move(request_),
					std::move(options_),
					nextLink
				);
			}

			[[nodiscard]] typename next_stream_response_type::type operator()(
				_In_ mref<typename next_stream_request_type::type> request_,
				_In_ mref<typename next_stream_request_type::options> options_
			) const noexcept override {
				return invoke_stage<feedback_stage_type, typename next_stream_response_type::type>(
					stage,
					std::move(request_),
					std::move(options_),
					nextLink
				);
			}
		};

		/**/

	private:
		TransformerChainLink<RequestType_, assets::IsStreamableAsset<RequestType_>> transformerLink;
		FeedbackChainLink<RequestType_, assets::IsStreamableAsset<RequestType_>> feedbackLink;
		CacheChainLink<RequestType_, assets::IsStreamableAsset<RequestType_>> cacheLink;
	};

	/* Loader Chain Template Selector */

	/**
	 * Non-Streamable template class
	 */
	template <
		IsRequestValueType RequestType_,
		IsResponseValueType ResponseType_ = ResourceBase,
		typename StageRefTypes_ = MetaTypeWrapperList<
			std::type_identity_t,
			std::type_identity_t,
			std::type_identity_t,
			sptr
		>,
		class CacheStageType_ = Cache<RequestType_, ResponseType_>,
		class FeedbackStageType_ = Feedback<RequestType_, ResponseType_>,
		class TransformerStageType_ = Transformer<RequestType_, ResponseType_>,
		class SourceLoaderType_ = SourceLoader,
		bool Streamable_ = assets::IsStreamableAsset<RequestType_>>
	class LoaderChain :
		public InheritMeta<
			LoaderChain<RequestType_, ResponseType_, StageRefTypes_, CacheStageType_, FeedbackStageType_,
				TransformerStageType_, SourceLoaderType_, Streamable_>,
			LoaderChainBase<RequestType_, ResponseType_, StageRefTypes_, CacheStageType_, FeedbackStageType_,
				TransformerStageType_, SourceLoaderType_>
		> {
	public:
		using this_type = LoaderChain<
			RequestType_,
			ResponseType_,
			StageRefTypes_,
			CacheStageType_,
			FeedbackStageType_,
			TransformerStageType_,
			SourceLoaderType_,
			Streamable_
		>;

		using base_type = LoaderChainBase<
			RequestType_,
			ResponseType_,
			StageRefTypes_,
			CacheStageType_,
			FeedbackStageType_,
			TransformerStageType_,
			SourceLoaderType_
		>;

		using traits = typename base_type::traits;

	public:
		template <
			typename CacheTx_,
			typename FeedbackTx_,
			typename TransformerTx_,
			typename SourceLoaderTx_
		>
		constexpr LoaderChain(
			CacheTx_&& cache_,
			FeedbackTx_&& feedback_,
			TransformerTx_&& transformer_,
			SourceLoaderTx_&& sourceLoader_
		) :
			InheritMeta<this_type, base_type>(
				std::forward<CacheTx_>(cache_),
				std::forward<FeedbackTx_>(feedback_),
				std::forward<TransformerTx_>(transformer_),
				std::forward<SourceLoaderTx_>(sourceLoader_)
			) {}

		~LoaderChain() override = default;

	public:
		[[nodiscard]] typename traits::response::type operator()(
			mref<typename traits::request::type> request_,
			mref<typename traits::request::options> options_
		) override {
			return base_type::template invoke_stage<typename base_type::cache_stage_type, typename
				traits::response::type>(
				base_type::cache,
				std::move(request_),
				std::move(options_),
				base_type::cacheLink
			);
		}
	};

	/**
	 * Streamable template class
	 */
	template <
		IsRequestValueType RequestType_,
		IsResponseValueType ResponseType_,
		typename StageRefTypes_,
		class CacheStageType_,
		class FeedbackStageType_,
		class TransformerStageType_,
		class SourceLoaderType_>
	class LoaderChain<
			RequestType_,
			ResponseType_,
			StageRefTypes_,
			CacheStageType_,
			FeedbackStageType_,
			TransformerStageType_,
			SourceLoaderType_,
			true
		> :
		public InheritMeta<LoaderChain<RequestType_, ResponseType_, StageRefTypes_, CacheStageType_, FeedbackStageType_,
			TransformerStageType_, SourceLoaderType_>, LoaderChainBase<
			RequestType_, ResponseType_, StageRefTypes_,
			CacheStageType_, FeedbackStageType_, TransformerStageType_, SourceLoaderType_>> {
	public:
		using this_type = LoaderChain<
			RequestType_,
			ResponseType_,
			StageRefTypes_,
			CacheStageType_,
			FeedbackStageType_,
			TransformerStageType_,
			SourceLoaderType_,
			true
		>;

		using base_type = LoaderChainBase<
			RequestType_,
			ResponseType_,
			StageRefTypes_,
			CacheStageType_,
			FeedbackStageType_,
			TransformerStageType_,
			SourceLoaderType_
		>;

		using traits = typename base_type::traits;

	public:
		template <
			typename CacheTx_,
			typename FeedbackTx_,
			typename TransformerTx_,
			typename SourceLoaderTx_
		>
		constexpr LoaderChain(
			CacheTx_&& cache_,
			FeedbackTx_&& feedback_,
			TransformerTx_&& transformer_,
			SourceLoaderTx_&& sourceLoader_
		) :
			InheritMeta<this_type, base_type>(
				std::forward<CacheTx_>(cache_),
				std::forward<FeedbackTx_>(feedback_),
				std::forward<TransformerTx_>(transformer_),
				std::forward<SourceLoaderTx_>(sourceLoader_)
			) {}

		~LoaderChain() override = default;

	public:
		[[nodiscard]] typename traits::response::type operator()(
			mref<typename traits::request::type> request_,
			mref<typename traits::request::options> options_
		) override {
			return base_type::template invoke_stage<typename base_type::cache_stage_type, typename
				traits::response::type>(
				base_type::cache,
				std::move(request_),
				std::move(options_),
				base_type::cacheLink
			);
		}

		[[nodiscard]] typename traits::stream_response::type operator()(
			mref<typename traits::stream_request::type> request_,
			mref<typename traits::stream_request::options> options_
		) override {
			return base_type::template invoke_stage<typename base_type::cache_stage_type, typename
				traits::stream_response::type>(
				base_type::cache,
				std::move(request_),
				std::move(options_),
				base_type::cacheLink
			);
		}
	};
}
