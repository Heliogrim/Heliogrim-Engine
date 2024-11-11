#pragma once

#include <Engine.Common/Functional/Lambda.hpp>
#include <Engine.Reflect/IsType.hpp>

#include "__fwd.hpp"
#include "../Action/DynamicStage.hpp"
#include "../Action/StaticStage.hpp"

/**/

// TODO: Maybe move into separate header
namespace hg::engine::storage::system {
	namespace {
		template <typename...>
		struct stage_fn_args {
			using type = stage_fn_args;
			using acq_input_type = void;
		};

		template <typename Context_, typename Storage_, typename Input_, typename... Rest_>
		struct stage_fn_args<Context_, Storage_, Input_, Rest_...> {
			using type = stage_fn_args<Context_, Storage_, Input_, Rest_...>;
			using acq_input_type = Input_;
		};

		template <typename Fn_>
		struct retrieve_fn_args;

		template <typename Fn_, typename... Args_>
		struct retrieve_fn_args<Fn_(Args_...)> : public stage_fn_args<Args_...> {};

		template <typename Fn_, typename... Args_>
		struct retrieve_fn_args<Fn_(Args_...) const> : public stage_fn_args<Args_...> {};

		template <typename ReturnType_, typename... Args_>
		struct retrieve_fn_args<ReturnType_(*)(Args_...)> : public stage_fn_args<Args_...> {};

		template <typename ReturnType_, typename Obj_, typename... Args_>
		struct retrieve_fn_args<ReturnType_(Obj_::*)(Args_...)> : public stage_fn_args<Args_...> {};

		template <typename ReturnType_, typename Obj_, typename... Args_>
		struct retrieve_fn_args<ReturnType_(Obj_::*)(Args_...) const> : public stage_fn_args<Args_...> {};

		template <typename Type_>
		concept IsFnObj = requires {
			{ &Type_::operator() };
		};

		template <typename Type_>
		struct is_fn_ptr_helper {
			constexpr static auto value = false;
		};

		template <typename ReturnType_, typename... Args_>
		struct is_fn_ptr_helper<ReturnType_ (*)(Args_...)> {
			constexpr static auto value = true;
		};

		template <typename Type_>
		concept IsFnPtr = is_fn_ptr_helper<Type_>::value;

		template <typename Fn_> requires IsFnObj<Fn_>
		struct retrieve_fn_args<Fn_> : public retrieve_fn_args<decltype(&Fn_::operator())> {};

		template <typename Fn_>
		using retrieve_fn_input_arg = typename retrieve_fn_args<Fn_>::acq_input_type;
	}
}

/**/

namespace hg::engine::storage::system {
	/**/

	class MorphStageType;
	class ComposeStageType;

	template <>
	struct StageResolutionFactory<void> {
		~StageResolutionFactory() = delete;
	};

	/**/

	template <std::same_as<DynamicStage> StageType_>
	struct StageResolutionFactory<StageType_> final {
	public:
		StageResolutionFactory(_Inout_ auto&& acqFn_, _Inout_ auto&& relFn_) noexcept
			requires std::is_constructible_v<decltype(DynamicStage::acqFn), decltype(acqFn_)> &&
			std::is_constructible_v<decltype(DynamicStage::relFn), decltype(relFn_)> :
			acqFnBase(::hg::forward<decltype(acqFn_)>(acqFn_)),
			relFnBase(::hg::forward<decltype(relFn_)>(relFn_)) {}

		constexpr ~StageResolutionFactory() = default;

		decltype(DynamicStage::acqFn) acqFnBase;
		decltype(DynamicStage::relFn) relFnBase;

		[[nodiscard]] DynamicStage operator()() const noexcept {
			return DynamicStage {
				.storage = nullptr,
				.acqFn = clone(acqFnBase),
				.access = {},
				.relFn = clone(relFnBase)
			};
		}
	};

	/**/

	template <>
	struct StageResolutionFactory<MorphStageType> {
	public:
		constexpr virtual ~StageResolutionFactory() noexcept = default;

		[[nodiscard]] virtual DynamicStage operator()() const noexcept = 0;
	};

	template <
		class StorageType_,
		typename AcqFn_,
		class ResourceType_,
		typename RelFn_
	>
	struct StageResolutionFactory<StaticStage<StorageType_, AcqFn_, ResourceType_, RelFn_>> final :
		public StageResolutionFactory<MorphStageType> {
	public:
		using this_type = StageResolutionFactory<StaticStage<StorageType_, AcqFn_, ResourceType_, RelFn_>>;

		using morph_acq_result_type = Result<meta::WithType<Rc<IoResourceAccessorBase>>, Action::acq_error_type>;

	public:
		[[nodiscard]] static morph_acq_result_type morphAcqFn(
			ref<Context> ctx_,
			cref<Arci<IStorage>> storage_,
			cref<meta::WithType<Rc<IoResourceAccessorBase>>> input_
		) {
			if (storage_ == nullptr || not IsType<StorageType_>(*storage_)) {
				return Unexpected<Action::acq_error_type> {
					IllegalTypeError { "Invalid storage supplied for morphed static stage acquisition." }
				};
			}

			/**/

			// Problem: We cannot deal with raw function pointers at the moment...
			static_assert(not IsFnPtr<AcqFn_>);

			/**/

			if constexpr (std::is_invocable_v<AcqFn_, ref<Context>, cref<Arci<StorageType_>>>) {

				const auto& storage = reinterpret_cast<cref<Arci<StorageType_>>>(storage_);
				Result<IoResourceAccessor<ResourceType_>, AccessError> intermediate = (AcqFn_ {})(ctx_, storage);

				return ::hg::move(intermediate).and_then(
					[](mref<IoResourceAccessor<ResourceType_>> accessor_) {
						return morph_acq_result_type::expected(
							meta::WithType<Rc<IoResourceAccessorBase>>::from(
								Rc<IoResourceAccessor<ResourceType_>>::create(::hg::move(accessor_))
							)
						);
					}
				);

			} else if constexpr (std::is_invocable_v<
				AcqFn_, ref<Context>, cref<Arci<StorageType_>>, retrieve_fn_input_arg<AcqFn_>
			>) {

				// Note: We will get `T`, `const T`, `T&`, `T&&`, `const T&` or `const T&&`, but resource constness
				// Note:	is propagated internally. Thus we can discard `cvref` of input resource type.
				using input_resource_type = retrieve_fn_input_arg<AcqFn_>;
				const auto& input = reinterpret_cast<cref<meta::WithType<Rc<std::remove_cvref_t<input_resource_type>>>>>(input_);

				if (input_.value == nullptr || not input.valid()) {
					return Unexpected<Action::acq_error_type> {
						IllegalTypeError { "Invalid input resource supplied for morphed static stage acquisition." }
					};
				}

				/**/

				const auto& storage = reinterpret_cast<cref<Arci<StorageType_>>>(storage_);

				// Attention: AcqFn_ ~ "Result_ (*)(Args_...)"
				Result<IoResourceAccessor<ResourceType_>, AccessError> intermediate = (AcqFn_ {})(ctx_, storage, *input.value);

				return ::hg::move(intermediate).and_then(
					[](mref<IoResourceAccessor<ResourceType_>> accessor_) {
						return morph_acq_result_type::expected(
							meta::WithType<Rc<IoResourceAccessorBase>>::from(
								Rc<IoResourceAccessor<ResourceType_>>::create(::hg::move(accessor_))
							)
						);
					}
				);
			} else {
				static_assert(not std::is_void_v<AcqFn_>, "Failed to reconstruct function arguments to morph static stage.");
				std::unreachable();
			}
		}

		static void morphRelFn(
			ref<Context> ctx_,
			cref<Arci<IStorage>> storage_,
			mref<meta::WithType<Rc<IoResourceAccessorBase>>> accessor_
		) noexcept {

			auto accessor = reinterpret_cast<mref<meta::WithType<Rc<IoResourceAccessor<ResourceType_>>>>>(accessor_);
			::hg::assertrt(accessor.valid());

			// Problem: We cannot deal with raw function pointers at the moment...
			static_assert(not IsFnPtr<RelFn_>);

			const auto& storage = reinterpret_cast<cref<Arci<StorageType_>>>(storage_);

			// Question: Should we protected against empty Rc<...> accessors?
			(RelFn_ {})(ctx_, storage, ::hg::move(*accessor.value));
			::hg::move(accessor).value.reset();
		}

	public:
		[[nodiscard]] DynamicStage operator()() const noexcept override {
			return DynamicStage {
				.storage = nullptr,
				.acqFn = &this_type::morphAcqFn,
				.access = {},
				.relFn = &this_type::morphRelFn
			};
		}
	};

	/**/

	template <>
	struct StageResolutionFactory<ComposeStageType> final {
	public:
		using this_type = StageResolutionFactory<ComposeStageType>;

		using compose_acq_result_type = Result<meta::WithType<Rc<IoResourceAccessorBase>>, Action::acq_error_type>;

		using stage_factory = Variant<
			nmpt<StageResolutionFactory<DynamicStage>>,
			nmpt<StageResolutionFactory<MorphStageType>>
		>;
		using substage_list = Vector<std::pair<refl::FullTypeInfo, stage_factory>>;

	public:
		constexpr explicit StageResolutionFactory(mref<substage_list> subStageFactories_) noexcept :
			subStageFactories(::hg::move(subStageFactories_)) {}

		constexpr ~StageResolutionFactory() noexcept = default;

	public:
		// Note: Composition targets variance / ambiguity of input resource type, therefore:
		// Attention: PartialTypeInfo of storage has to be exactly the same
		// Attention: FullTypeInfo of resource has to be exactly the same

		substage_list subStageFactories;

	protected:
		struct SubStage {
			refl::FullTypeInfo inputTypeInfo;
			decltype(DynamicStage::acqFn) acqFn;
			decltype(DynamicStage::relFn) relFn;
		};

		[[nodiscard]] static compose_acq_result_type composeAcqFn(
			_Inout_ ref<Vector<SubStage>> subStages_,
			ref<Context> ctx_,
			cref<Arci<IStorage>> storage_,
			cref<meta::WithType<Rc<IoResourceAccessorBase>>> input_
		) noexcept {

			if (storage_ == nullptr) {
				return Unexpected {
					Action::acq_error_type { IllegalArgumentError { "Null storage supplied for composed stage acquisition." } }
				};
			}

			::hg::assertd(not subStages_.empty());

			/**/

			compose_acq_result_type result = Unexpected {
				Action::acq_error_type { IllegalTypeError { "Failed to find composed sub stage matching type requirements." } }
			};
			const auto end = subStages_.end();
			for (auto iter = subStages_.begin(); iter != end; ++iter) {

				// TODO: Extend reflection or With<TypeTrait, ...> to support `is<T>` checks
				// Problem: The provided inputTypeInfo corresponds to the actual resource type
				// Problem:		therefore we are not able to pre-check the meta type id.
				// Problem: `TypeId< Rc<Accessor< T >> >` vs `TypeId< T >`

				//if (
				//	(iter->inputTypeInfo.meta != nullptr && input_.value == nullptr) ||
				//	(input_.typeId != iter->inputTypeInfo.meta->typeId())
				//) {
				//	continue;
				//}

				if (iter->inputTypeInfo.meta != nullptr && input_.value == nullptr) {
					continue;
				}

				/**/

				result = iter->acqFn(ctx_, storage_, input_);

				if (result.has_value()) {
					// Note: We move the applied function to the front, so we know what function to call when releasing
					std::swap(subStages_.front(), *iter);
					return result;
				}

				// Note: We expect `IllegalArgumentError` and `IllegalTypeError`, due to the brute-force probing
				if (not result.error().is<IllegalArgumentError>() && not result.error().is<IllegalTypeError>()) {
					return result;
				}
			}

			/**/

			return result;
		}

		static void composeRelFn(
			_In_ cref<Vector<SubStage>> subStages_,
			ref<Context> ctx_,
			cref<Arci<IStorage>> storage_,
			mref<meta::WithType<Rc<IoResourceAccessorBase>>> accessor_
		) noexcept {
			::hg::assertd(not subStages_.empty());
			subStages_.front().relFn(ctx_, storage_, ::hg::move(accessor_));
		}

	public:
		[[nodiscard]] DynamicStage operator()() const noexcept {

			// TODO: Replace!?! Maybe we need some kind of 'next-object' chain at dynamic stage
			auto subStages = Vector<SubStage> {};
			subStages.reserve(subStageFactories.size());
			for (const auto& subStageFactory : subStageFactories) {
				subStageFactory.second.apply(
					Overloaded {
						[&subStages, &subStageFactory](nmpt<StageResolutionFactory<DynamicStage>> factory_) mutable {
							subStages.emplace_back(
								subStageFactory.first,
								clone(factory_->acqFnBase),
								clone(factory_->relFnBase)
							);
						},
						[&subStages, &subStageFactory](nmpt<StageResolutionFactory<MorphStageType>> factory_) mutable {
							// Attention: This is just a workaround, as we are currently unable to retrieve the static function pointers
							const auto dummy = (*factory_)();
							subStages.emplace_back(
								subStageFactory.first,
								::hg::move(dummy.acqFn),
								::hg::move(dummy.relFn)
							);
						}
					}
				);
			}

			auto rcSubStages = Rc<Vector<SubStage>>::create(::hg::move(subStages));

			return DynamicStage {
				.storage = nullptr,
				.acqFn = [rcSubStages = clone(rcSubStages)](auto&&... args_) mutable {
					return this_type::composeAcqFn(*rcSubStages, ::hg::forward<decltype(args_)>(args_)...);
				},
				.access = {},
				.relFn = [rcSubStages = ::hg::move(rcSubStages)](auto&&... args_) mutable {
					return this_type::composeRelFn(*rcSubStages, ::hg::forward<decltype(args_)>(args_)...);
				}
			};
		}
	};
}
