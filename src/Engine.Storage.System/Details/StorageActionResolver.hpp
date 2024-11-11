#pragma once

#include <concepts>
#include <Engine.Storage.Action/Store/ActionTypeResolutionCache.hpp>
#include <Engine.Storage.Action/Store/StageTypeResolutionStore.hpp>

namespace hg::engine::storage {
	class StorageActionResolver {
	public:
		using this_type = StorageActionResolver;

	public:
		StageTypeResolutionStore stageStore;
		ActionTypeResolutionCache actionCache;

	private:
		[[nodiscard]] Opt<cref<std::pair<StageTypeInfo, UniquePtr<system::StageResolutionFactory<system::ComposeStageType>>>>>
		buildComposedStage(
			_In_ cref<StageTypeInfo> actionTypeInfo_,
			_In_ cref<StageTypeInfo> providingTypeInfo_
		) const noexcept;

		[[nodiscard]] Opt<ref<system::ActionResolutionFactory<Mutation>>> buildMutation(
			_In_ cref<ActionTypeInfo> actionTypeInfo_
		) const noexcept;

		[[nodiscard]] Opt<ref<system::ActionResolutionFactory<Query>>> buildQuery(
			_In_ cref<ActionTypeInfo> actionTypeInfo_
		) const noexcept;

		void invalidateMutation(_In_ cref<ActionTypeInfo> actionTypeInfo_) const noexcept;

		void invalidateQuery(_In_ cref<ActionTypeInfo> actionTypeInfo_) const noexcept;

	public:
		[[nodiscard]] Opt<ref<system::ActionResolutionFactory<Query>>> resolveQuery(
			_In_ cref<ActionTypeInfo> actionTypeInfo_
		) const noexcept;

		[[nodiscard]] Opt<ref<system::ActionResolutionFactory<Mutation>>> resolveMutation(
			_In_ cref<ActionTypeInfo> actionTypeInfo_
		) const noexcept;

	private:
		void use(
			_Inout_ mref<ActionTypeInfo> actionTypeInfo_,
			_Inout_ mref<UniquePtr<system::ActionResolutionFactory<Query>>> factory_
		);

		void use(
			_Inout_ mref<ActionTypeInfo> actionTypeInfo_,
			_Inout_ mref<UniquePtr<system::ActionResolutionFactory<Mutation>>> factory_
		);

	public:
		template <CompleteType ActionType_> requires std::derived_from<ActionType_, Query> &&
			std::is_default_constructible_v<ActionType_>
		void use(_Inout_ mref<ActionTypeInfo> actionTypeInfo_) {
			auto factory = make_uptr<system::ActionResolutionFactory<Query, ActionType_>>();
			use(::hg::move(actionTypeInfo_), ::hg::move(factory));
		}

		template <CompleteType ActionType_> requires std::derived_from<ActionType_, Mutation> &&
			std::is_default_constructible_v<ActionType_>
		void use(_Inout_ mref<ActionTypeInfo> actionTypeInfo_) {
			auto factory = make_uptr<system::ActionResolutionFactory<Mutation, ActionType_>>();
			use(::hg::move(actionTypeInfo_), ::hg::move(factory));
		}

	private:
		// TODO: add function to store different stages to resolver -> store/cache
		void useStage(
			_Inout_ mref<StageTypeInfo> stageTypeInfo_,
			_Inout_ mref<UniquePtr<system::StageResolutionFactory<system::DynamicStage>>> factory_
		);

		void useStage(
			_Inout_ mref<StageTypeInfo> stageTypeInfo_,
			_Inout_ mref<UniquePtr<system::StageResolutionFactory<system::MorphStageType>>> factory_
		);

	public:
		template <CompleteType StageType_> requires std::is_same_v<StageType_, system::DynamicStage>
		void useStage(_Inout_ mref<StageTypeInfo> stageTypeInfo_, _In_ auto&& acqFn_, _In_ auto&& relFn_) {
			auto factory = make_uptr<system::StageResolutionFactory<StageType_>>(
				::hg::forward<decltype(acqFn_)>(acqFn_),
				::hg::forward<decltype(relFn_)>(relFn_)
			);
			useStage(::hg::move(stageTypeInfo_), ::hg::move(factory));
		}

		template <CompleteType StageType_> requires (not std::is_same_v<StageType_, system::DynamicStage>) &&
			std::is_default_constructible_v<StageType_>
		void useStage(_Inout_ mref<StageTypeInfo> stageTypeInfo_) {
			auto factory = make_uptr<system::StageResolutionFactory<StageType_>>();
			useStage(::hg::move(stageTypeInfo_), ::hg::move(factory));
		}

	public:
		template <class StorageType_, class InputResourceType_, class AccessResourceType_>
		struct StaticResolver;

		template <class StorageType_, class InputResourceType_, class AccessResourceType_>
		decltype(auto) fetchStaticStage() {
			constexpr auto resolver = StaticResolver<StorageType_, InputResourceType_, AccessResourceType_> {};
			return resolver(stageStore);
		}

	public:
		template <class StorageType_, class InputResourceType_, class ResourceType_>
		struct DynamicResolver;

		template <typename ResourceType_>
		decltype(auto) fetchDynamicStage() {
			constexpr auto resolver = DynamicResolver<void, void, ResourceType_> {};
			return resolver(stageStore);
		}

		template <
			class StorageType_,
			class InputResourceType_,
			class AccessResourceType_
		>
		decltype(auto) fetchDynamicStage() {
			constexpr auto resolver = DynamicResolver<StorageType_, InputResourceType_, AccessResourceType_> {};
			return resolver(stageStore);
		}
	};
}
