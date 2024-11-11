#include "StorageActionResolver.hpp"

#include <Engine.Asserts/Todo.hpp>
#include <Engine.Pedantic/Into/Into.hpp>

using namespace hg::engine::storage::system;
using namespace hg::engine::storage;
using namespace hg::engine;
using namespace hg;

/**/

[[nodiscard]] static auto findInCache(
	_In_ auto&& cache_,
	_In_ cref<ActionTypeInfo> actionTypeInfo_
) -> std::conditional_t<
	std::is_const_v<std::remove_reference_t<decltype(cache_)>>,
	typename std::remove_cvref_t<decltype(cache_)>::const_iterator,
	typename std::remove_cvref_t<decltype(cache_)>::iterator
>;

[[nodiscard]] static auto findInStore(
	_In_ auto&& store_,
	_In_ cref<StageTypeInfo> stageTypeInfo_
) -> std::conditional_t<
	std::is_const_v<std::remove_reference_t<decltype(store_)>>,
	typename std::remove_cvref_t<decltype(store_)>::const_iterator,
	typename std::remove_cvref_t<decltype(store_)>::iterator
>;

[[nodiscard]] static auto findCovariantInStore(
	_In_ const auto& storeBegin_,
	_In_ const auto& storeEnd_,
	_In_ cref<StageTypeInfo> stageTypeInfo_
) -> std::remove_cvref_t<decltype(storeBegin_)>;

/**/

Opt<cref<std::pair<StageTypeInfo, UniquePtr<StageResolutionFactory<ComposeStageType>>>>> StorageActionResolver::buildComposedStage(
	cref<StageTypeInfo> actionTypeInfo_,
	cref<StageTypeInfo> providingTypeInfo_
) const noexcept {

	// Warning: `actionTypeInfo_.input` is stained by the previously failing resolver cycle
	// Warning: `providingTypeInfo_.resource` is stained by the previously failing resolver cycle

	[[maybe_unused]] const auto& stageStorage = actionTypeInfo_.storage;
	[[maybe_unused]] const auto& stageResource = actionTypeInfo_.resource;
	[[maybe_unused]] const auto& nextStorage = providingTypeInfo_.storage;

	// Find all possible stages corresponding to the next storage type
	Vector<StageTypeInfo> nextLevelCandidates {};
	for (const auto& stage : stageStore.dynamicResolved) {
		if (nextStorage.is(stage.first.storage)) {
			nextLevelCandidates.emplace_back(stage.first);
		}
	}

	for (const auto& stage : stageStore.morphingResolved) {
		if (nextStorage.is(stage.first.storage)) {
			nextLevelCandidates.emplace_back(stage.first);
		}
	}

	if (nextLevelCandidates.empty()) {
		return None;
	}

	// Find all possible stages complying with at least a covariant resource exposure, and covariant input type

	// Note: We do not allow recursive composed stage factories or stage reconstruction
	using stage_factory_type = StageResolutionFactory<ComposeStageType>::stage_factory;
	Vector<std::pair<refl::FullTypeInfo, stage_factory_type>> covariantCandidates {};

	auto nextLevelIter = nextLevelCandidates.begin();
	const auto nextLevelEnd = nextLevelCandidates.end();
	for (; nextLevelIter != nextLevelEnd; ++nextLevelIter) {

		const auto& checkTypeInfo = *nextLevelIter;
		for (const auto& stage : stageStore.dynamicResolved) {

			const auto& stageTypeInfo = stage.first;
			if (
				stageStorage.is(stageTypeInfo.storage) &&
				stageTypeInfo.resource.is(stageResource) &&
				stageTypeInfo.input.is(checkTypeInfo.resource)
			) {
				// Note: We only need the covariant input type @see StageResolutionFactory<ComposeStageType>::subStageFactories
				covariantCandidates.emplace_back(stageTypeInfo.input, stage.second.get());
			}
		}

		for (const auto& stage : stageStore.morphingResolved) {

			const auto& stageTypeInfo = stage.first;
			if (
				stageStorage.is(stageTypeInfo.storage) &&
				stageTypeInfo.resource.is(stageResource) &&
				stageTypeInfo.input.is(checkTypeInfo.resource)
			) {
				// Note: We only need the covariant input type @see StageResolutionFactory<ComposeStageType>::subStageFactories
				covariantCandidates.emplace_back(stageTypeInfo.input, stage.second.get());
			}
		}

		/**/

		if (not covariantCandidates.empty()) {
			break;
		}
	}

	/**/

	if (covariantCandidates.empty()) {
		return None;
	}

	// If we have at least one valid next level candidate and corresponding stages to compose the previous stage
	//	we can construct a replacing stage for the current target.

	auto composedTypeInfo = StageTypeInfo {
		.storage = stageStorage,
		.resource = stageResource,
		.input = nextLevelCandidates.front().resource
	};
	auto factory = make_uptr<StageResolutionFactory<ComposeStageType>>(::hg::move(covariantCandidates));

	/**/

	const auto& stored = const_cast<ref<this_type>>(*this).stageStore.composingResolved.emplace_back(
		::hg::move(composedTypeInfo),
		::hg::move(factory)
	);
	return Some(stored);
}

Opt<ref<ActionResolutionFactory<Mutation>>> StorageActionResolver::buildMutation(
	cref<ActionTypeInfo> actionTypeInfo_
) const noexcept {

	using action_factory_type = ActionResolutionFactory<Mutation, MutationDynamicGen>;

	/**/

	// Problem: This is just a naive implementation, which is likely to break on slightly ambiguous stage definitions

	auto actionTypeInfo = clone(actionTypeInfo_);
	actionTypeInfo.stageTypeInfos.front().input = { nullptr };
	actionTypeInfo.stageTypeInfos.front().resource = actionTypeInfo_.targetResourceInfo;

	action_factory_type::inline_factories stageFactories {};
	for (auto idx = 0uLL; idx < actionTypeInfo.stageTypeInfos.size(); ++idx) {

		auto& stageInfo = *(actionTypeInfo.stageTypeInfos.begin() + idx);

		/**/

		if (idx != 0uLL) {
			const auto& prevStageInfo = *(actionTypeInfo.stageTypeInfos.begin() + idx - 1uLL);

			stageInfo.input = { nullptr };
			stageInfo.resource = prevStageInfo.input;
		}

		/**/

		auto compliant = findCovariantInStore(
			stageStore.dynamicResolved.begin(),
			stageStore.dynamicResolved.end(),
			stageInfo
		);

		if (compliant != stageStore.dynamicResolved.end()) {

			stageInfo.input = compliant->first.input;
			stageInfo.resource = compliant->first.resource;
			stageInfo.storage = compliant->first.storage;

			stageFactories.emplace_back(compliant->second.get());
			continue;
		}

		/**/

		auto morphCompliant = findCovariantInStore(
			stageStore.morphingResolved.begin(),
			stageStore.morphingResolved.end(),
			stageInfo
		);

		if (morphCompliant != stageStore.morphingResolved.end()) {

			stageInfo.input = morphCompliant->first.input;
			stageInfo.resource = morphCompliant->first.resource;
			stageInfo.storage = morphCompliant->first.storage;

			stageFactories.emplace_back(morphCompliant->second.get());
			continue;
		}

		/**/

		auto composeCompliant = findCovariantInStore(
			stageStore.composingResolved.begin(),
			stageStore.composingResolved.end(),
			stageInfo
		);

		if (composeCompliant != stageStore.composingResolved.end()) {

			stageInfo.input = composeCompliant->first.input;
			stageInfo.resource = composeCompliant->first.resource;
			stageInfo.storage = composeCompliant->first.storage;

			stageFactories.emplace_back(composeCompliant->second.get());
			continue;
		}

		/**/

		if (idx != 0uLL) {

			// Note: When we failed to resolve the current stage with stageInfo, we may need to use a composition stage
			// Note:	at the previous level, so we may be able to find a followup / providing stage in store.

			auto& prevStageInfo = *(actionTypeInfo.stageTypeInfos.begin() + idx - 1LL);
			auto maybeComposed = buildComposedStage(prevStageInfo, stageInfo);

			if (maybeComposed.has_value()) {

				// Note: Replace previous stored stage, as we are not able to fully resolve the chain otherwise.

				prevStageInfo.input = maybeComposed->first.input;
				prevStageInfo.resource = maybeComposed->first.resource;
				prevStageInfo.storage = maybeComposed->first.storage;

				stageFactories.back() = maybeComposed->second.get();

				// Note: Lower the current index to repeat the resolution of the follow-up stage.
				--idx;

				stageInfo.input = { nullptr };
				stageInfo.resource = prevStageInfo.input;

				continue;
			}

		}

		/**/

		return None;
	}

	/**/

	actionTypeInfo.targetResourceInfo = actionTypeInfo.stageTypeInfos.front().resource;

	/**/

	auto factory = make_uptr<ActionResolutionFactory<Mutation, MutationDynamicGen>>(::hg::move(stageFactories));

	auto& storedFactory = static_cast<ActionResolutionFactory<Mutation>&>(*factory);
	const_cast<ref<this_type>>(*this).use(::hg::move(actionTypeInfo), ::hg::move(factory));

	return Some(storedFactory);
}

Opt<ref<ActionResolutionFactory<Query>>> StorageActionResolver::buildQuery(
	cref<ActionTypeInfo> actionTypeInfo_
) const noexcept {

	using action_factory_type = ActionResolutionFactory<Query, QueryDynamicGen>;

	/**/

	// Problem: This is just a naive implementation, which is likely to break on slightly ambiguous stage definitions

	auto actionTypeInfo = clone(actionTypeInfo_);
	actionTypeInfo.stageTypeInfos.front().input = { nullptr };
	actionTypeInfo.stageTypeInfos.front().resource = actionTypeInfo_.targetResourceInfo;

	action_factory_type::inline_factories stageFactories {};
	for (auto idx = 0uLL; idx < actionTypeInfo.stageTypeInfos.size(); ++idx) {

		auto& stageInfo = *(actionTypeInfo.stageTypeInfos.begin() + idx);

		/**/

		if (idx != 0uLL) {
			const auto& prevStageInfo = *(actionTypeInfo.stageTypeInfos.begin() + idx - 1uLL);

			stageInfo.input = { nullptr };
			stageInfo.resource = prevStageInfo.input;
		}

		/**/

		auto compliant = findCovariantInStore(
			stageStore.dynamicResolved.begin(),
			stageStore.dynamicResolved.end(),
			stageInfo
		);

		if (compliant != stageStore.dynamicResolved.end()) {

			stageInfo.input = compliant->first.input;
			stageInfo.resource = compliant->first.resource;
			stageInfo.storage = compliant->first.storage;

			stageFactories.emplace_back(compliant->second.get());
			continue;
		}

		/**/

		auto morphCompliant = findCovariantInStore(
			stageStore.morphingResolved.begin(),
			stageStore.morphingResolved.end(),
			stageInfo
		);

		if (morphCompliant != stageStore.morphingResolved.end()) {

			stageInfo.input = morphCompliant->first.input;
			stageInfo.resource = morphCompliant->first.resource;
			stageInfo.storage = morphCompliant->first.storage;

			stageFactories.emplace_back(morphCompliant->second.get());
			continue;
		}

		/**/

		auto composeCompliant = findCovariantInStore(
			stageStore.composingResolved.begin(),
			stageStore.composingResolved.end(),
			stageInfo
		);

		if (composeCompliant != stageStore.composingResolved.end()) {

			stageInfo.input = composeCompliant->first.input;
			stageInfo.resource = composeCompliant->first.resource;
			stageInfo.storage = composeCompliant->first.storage;

			stageFactories.emplace_back(composeCompliant->second.get());
			continue;
		}

		/**/

		if (idx != 0uLL) {

			// Note: When we failed to resolve the current stage with stageInfo, we may need to use a composition stage
			// Note:	at the previous level, so we may be able to find a followup / providing stage in store.

			auto& prevStageInfo = *(actionTypeInfo.stageTypeInfos.begin() + idx - 1LL);
			auto maybeComposed = buildComposedStage(prevStageInfo, stageInfo);

			if (maybeComposed.has_value()) {

				// Note: Replace previous stored stage, as we are not able to fully resolve the chain otherwise.

				prevStageInfo.input = maybeComposed->first.input;
				prevStageInfo.resource = maybeComposed->first.resource;
				prevStageInfo.storage = maybeComposed->first.storage;

				stageFactories.back() = maybeComposed->second.get();

				// Note: Lower the current index to repeat the resolution of the follow-up stage.
				--idx;

				stageInfo.input = { nullptr };
				stageInfo.resource = prevStageInfo.input;

				continue;
			}

		}

		/**/

		return None;
	}

	/**/

	actionTypeInfo.targetResourceInfo = actionTypeInfo.stageTypeInfos.front().resource;

	/**/

	auto factory = make_uptr<ActionResolutionFactory<Query, QueryDynamicGen>>(::hg::move(stageFactories));

	auto& storedFactory = static_cast<ActionResolutionFactory<Query>&>(*factory);
	const_cast<ref<this_type>>(*this).use(::hg::move(actionTypeInfo), ::hg::move(factory));

	return Some(storedFactory);
}

void StorageActionResolver::invalidateQuery(cref<ActionTypeInfo> actionTypeInfo_) const noexcept {
	::hg::todo_panic();
}

void StorageActionResolver::invalidateMutation(cref<ActionTypeInfo> actionTypeInfo_) const noexcept {
	::hg::todo_panic();
}

Opt<ref<ActionResolutionFactory<Query>>> StorageActionResolver::resolveQuery(
	_In_ cref<ActionTypeInfo> actionTypeInfo_
) const noexcept {

	const auto existing = findInCache(actionCache.queryActions, actionTypeInfo_);
	if (existing != actionCache.queryActions.end()) {
		return Some(*existing->second);
	}

	// TODO:

	// Note: If we have a cache break, we need to build a new action to comply with the type requirements
	return buildQuery(actionTypeInfo_);
}

[[nodiscard]] Opt<ref<ActionResolutionFactory<Mutation>>> StorageActionResolver::resolveMutation(
	_In_ cref<ActionTypeInfo> actionTypeInfo_
) const noexcept {

	const auto existing = findInCache(actionCache.mutationActions, actionTypeInfo_);
	if (existing != actionCache.mutationActions.end()) {
		return Some(*existing->second);
	}

	// Note: If we have a cache break, we need to build a new action to comply with the type requirements
	return buildMutation(actionTypeInfo_);
}

void StorageActionResolver::use(mref<ActionTypeInfo> actionTypeInfo_, mref<UniquePtr<ActionResolutionFactory<Query>>> factory_) {

	const auto existing = findInCache(actionCache.queryActions, actionTypeInfo_);
	if (existing != actionCache.queryActions.end()) {
		existing->second = ::hg::move(factory_);
		return;
	}

	/**/

	actionCache.queryActions.emplace_back(::hg::move(actionTypeInfo_), ::hg::move(factory_));
}

void StorageActionResolver::use(mref<ActionTypeInfo> actionTypeInfo_, mref<UniquePtr<ActionResolutionFactory<Mutation>>> factory_) {

	const auto existing = findInCache(actionCache.mutationActions, actionTypeInfo_);
	if (existing != actionCache.mutationActions.end()) {
		existing->second = ::hg::move(factory_);
		return;
	}

	/**/

	actionCache.mutationActions.emplace_back(::hg::move(actionTypeInfo_), ::hg::move(factory_));
}

void StorageActionResolver::useStage(
	mref<StageTypeInfo> stageTypeInfo_,
	mref<UniquePtr<StageResolutionFactory<DynamicStage>>> factory_
) {

	const auto existing = findInStore(stageStore.dynamicResolved, stageTypeInfo_);
	if (existing != stageStore.dynamicResolved.end()) {
		existing->second = ::hg::move(factory_);
		return;
	}

	/**/

	stageStore.dynamicResolved.emplace_back(::hg::move(stageTypeInfo_), ::hg::move(factory_));
}

void StorageActionResolver::useStage(
	mref<StageTypeInfo> stageTypeInfo_,
	mref<UniquePtr<StageResolutionFactory<MorphStageType>>> factory_
) {

	const auto existing = findInStore(stageStore.morphingResolved, stageTypeInfo_);
	if (existing != stageStore.morphingResolved.end()) {
		existing->second = ::hg::move(factory_);
		return;
	}

	/**/

	stageStore.morphingResolved.emplace_back(::hg::move(stageTypeInfo_), ::hg::move(factory_));
}

/**/

auto findInCache(
	auto&& cache_,
	cref<ActionTypeInfo> actionTypeInfo_
) -> std::conditional_t<
	std::is_const_v<std::remove_reference_t<decltype(cache_)>>,
	typename std::remove_cvref_t<decltype(cache_)>::const_iterator,
	typename std::remove_cvref_t<decltype(cache_)>::iterator
> {
	return std::ranges::find_if(
		cache_,
		[&actionTypeInfo_](const auto& cached_) {

			// Check `Input` and `Output` of action
			if (
				not actionTypeInfo_.baseStorageInfo().is(cached_.first.baseStorageInfo()) ||
				not cached_.first.targetResourceInfo.is(actionTypeInfo_.targetResourceInfo)
			) {
				return false;
			}

			// Check intermediate steps for possible conflicts
			if (cached_.first.stageTypeInfos.size() != actionTypeInfo_.stageTypeInfos.size()) {
				return false;
			}

			auto citer = cached_.first.stageTypeInfos.begin();
			auto aiter = actionTypeInfo_.stageTypeInfos.begin();

			const auto end = cached_.first.stageTypeInfos.end();
			for (; citer != end; ++citer, ++aiter) {
				// We can only pre-fetch the storage types, as everything else is resolved at `runtime`
				if (not aiter->storage.is(citer->storage)) {
					return false;
				}
			}

			return true;
		}
	);
}

auto findInStore(
	auto&& store_,
	cref<StageTypeInfo> stageTypeInfo_
) -> std::conditional_t<
	std::is_const_v<std::remove_reference_t<decltype(store_)>>,
	typename std::remove_cvref_t<decltype(store_)>::const_iterator,
	typename std::remove_cvref_t<decltype(store_)>::iterator
> {
	return std::ranges::find_if(
		store_,
		[&stageTypeInfo_](const auto& stored_) {

			// Check for full type equality regarding inputs, outputs, and consumed storage types
			return stageTypeInfo_.input.exact(stored_.first.input) &&
				stageTypeInfo_.resource.exact(stored_.first.resource) &&
				stageTypeInfo_.storage.exact(stored_.first.storage);
		}
	);
}

auto findCovariantInStore(
	const auto& storeBegin_,
	const auto& storeEnd_,
	cref<StageTypeInfo> stageTypeInfo_
) -> std::remove_cvref_t<decltype(storeBegin_)> {

	// Note: This is just for temporary safety; We expect in the current application to not know the input resource type.
	::hg::assertrt(stageTypeInfo_.input.meta == nullptr);

	::hg::assertrt(
		stageTypeInfo_.resource.meta != nullptr &&
		stageTypeInfo_.storage.meta != nullptr
	);

	/**/

	for (auto iter = storeBegin_; iter != storeEnd_; ++iter) {

		const auto& typeInfo = iter->first;
		if (not stageTypeInfo_.storage.is(typeInfo.storage)) {
			continue;
		}

		if (not typeInfo.resource.is(stageTypeInfo_.resource)) {
			continue;
		}

		return iter;
	}

	return storeEnd_;
}
