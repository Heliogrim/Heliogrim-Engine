#pragma once

#include <Engine.Common/Make.hpp>
#include <Engine.Common/Variant.hpp>
#include <Engine.Common/Memory/UniquePointer.hpp>
#include <Engine.Common/Meta/IsAnyOf.hpp>

#include "__fwd.hpp"
#include "../Action/MutationDynamicGen.hpp"
#include "../Action/QueryDynamicGen.hpp"

namespace hg::engine::storage::system {
	class ComposeStageType;
	class MorphStageType;
}

namespace hg::engine::storage::system {
	template <class ActionType_>
	struct ActionResolutionFactory<ActionType_, ActionType_> {
		constexpr virtual ~ActionResolutionFactory() = default;

		[[nodiscard]] virtual UniquePtr<ActionType_> operator()() const noexcept = 0;
	};

	template <class ActionType_, std::derived_from<ActionType_> InstantiationType_>
		requires IsAnyOf<InstantiationType_, MutationDynamicGen, QueryDynamicGen>
	struct ActionResolutionFactory<ActionType_, InstantiationType_> :
		public ActionResolutionFactory<ActionType_> {
	public:
		using this_type = ActionResolutionFactory<ActionType_, InstantiationType_>;

		using stage_factory = std::conditional_t<
			IsAnyOf<InstantiationType_, MutationDynamicGen, QueryDynamicGen>,
			Variant<
				nmpt<StageResolutionFactory<DynamicStage>>,
				nmpt<StageResolutionFactory<MorphStageType>>,
				nmpt<StageResolutionFactory<ComposeStageType>>>,
			void>;
		using inline_factories = InlineAutoArray<stage_factory, 3>;

	public:
		constexpr explicit ActionResolutionFactory(_Inout_ mref<inline_factories> stageFactories_) noexcept :
			ActionResolutionFactory<ActionType_, ActionType_>(),
			stageFactories(::hg::move(stageFactories_)) {}

		ActionResolutionFactory(const this_type&) = delete;

		ActionResolutionFactory(this_type&&) noexcept = default;

		~ActionResolutionFactory() override = default;

	public:
		ref<this_type> operator=(cref<this_type>) = delete;

		ref<this_type> operator=(mref<this_type>) noexcept = default;

	public:
		inline_factories stageFactories;

		[[nodiscard]] UniquePtr<ActionType_> operator()() const noexcept override {

			InlineAutoArray<DynamicStage, 3> stages {};
			for (const auto& factoryVariant : stageFactories) {
				factoryVariant.apply(
					[&stages](const auto& factory_) {
						stages.emplace_back(std::invoke(*factory_));
					}
				);
			}

			return make_uptr<InstantiationType_>(::hg::move(stages));
		}
	};

	template <class ActionType_, std::derived_from<ActionType_> InstantiationType_>
		requires (not IsAnyOf<InstantiationType_, MutationDynamicGen, QueryDynamicGen>)
	struct ActionResolutionFactory<ActionType_, InstantiationType_> final :
		public ActionResolutionFactory<ActionType_> {
		constexpr ~ActionResolutionFactory() override = default;

		[[nodiscard]] UniquePtr<ActionType_> operator()() const noexcept override {
			return make_uptr<InstantiationType_>();
		}
	};
}
