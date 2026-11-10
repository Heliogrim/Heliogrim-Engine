#include "ExtendedStorageSystem.hpp"

#include <Engine.Async/Lock/SharedLock.hpp>
#include <Engine.Async/Lock/UniqueLock.hpp>
#include <Engine.Storage.Action/Access/AccessError.hpp>

using namespace hg::engine::storage;
using namespace hg;

namespace {
	/* Declared after the stack lock, so the chain is dropped before the lock opens, on every exit path. */
	struct InvalidateOnExit {
		ref<StackCache> stacks;
		ptr<const IStorage> primary;

		~InvalidateOnExit() {
			stacks.invalidateFrom(primary);
		}
	};

	/* Declared after the stack lock, so `_top` is dropped before the lock opens, even if `exec` throws. */
	template <typename Action_, typename Context_>
	struct ReleaseOnExit {
		ref<Action_> action;
		ref<Context_> context;

		~ReleaseOnExit() {
			action.release(context);
		}
	};

	[[nodiscard]] AccessError retiredError() {
		return AccessError { "Storage was removed from the storage system." };
	}
}

Result<void, IoMutationGuard<void>::error_type> ExtendedStorageSystem::execMutation(
	ref<Mutation> query_,
	mref<std::function<void(cref<IoResourceAccessorBase>)>> mutationFn_
) {

	auto& context = _context;

	// shortcut: a storage call nested in another storage callback on the same primary re-takes this FiberSharedMutex on the same fiber and deadlocks once a writer waits (immediately for a mutation nested in a query), upgrade to a per-fiber owner check if nested calls appear.
	::hg::assertrt(query_.primary() != nullptr);
	const auto stackLock = concurrent::UniqueLock { context.stacks.lockOf(query_.primary()) };
	if (query_.primary()->isRetired()) {
		return Unexpected { IoMutationGuard<void>::error_type { retiredError() } };
	}

	// Note: Mutation stages are not cached; invalidate even on failure or throw, as a partial commit may have changed the data.
	const auto invalidate = InvalidateOnExit { .stacks = context.stacks, .primary = query_.primary() };

	/**/

	// Warning: Should have been called already while `fetching` query_.prepare()
	auto acqResult = query_.acquire(context);
	if (not acqResult.has_value()) {
		return Unexpected { IoMutationGuard<void>::error_type { ::hg::move(acqResult).error() } };
	}

	query_.exec(::hg::move(mutationFn_));

	/* auto relResult = */
	query_.release(context);
	// Warning: query_.cleanup() should be auto called after guard release.

	return {};
}

Result<Mutation::exec_valreturn_type, IoMutationGuard<void>::error_type> ExtendedStorageSystem::execMutation(
	ref<Mutation> mutation_,
	mref<std::function<Mutation::exec_valreturn_type(cref<IoResourceAccessorBase>)>> mutationFn_
) {

	auto& context = _context;

	// shortcut: a storage call nested in another storage callback on the same primary re-takes this FiberSharedMutex on the same fiber and deadlocks once a writer waits (immediately for a mutation nested in a query), upgrade to a per-fiber owner check if nested calls appear.
	::hg::assertrt(mutation_.primary() != nullptr);
	const auto stackLock = concurrent::UniqueLock { context.stacks.lockOf(mutation_.primary()) };
	if (mutation_.primary()->isRetired()) {
		return Unexpected { IoMutationGuard<void>::error_type { retiredError() } };
	}

	// Note: Mutation stages are not cached; invalidate even on failure or throw, as a partial commit may have changed the data.
	const auto invalidate = InvalidateOnExit { .stacks = context.stacks, .primary = mutation_.primary() };

	/**/

	auto acqResult = mutation_.acquire(context);
	if (not acqResult.has_value()) {
		return Unexpected { IoMutationGuard<void>::error_type { ::hg::move(acqResult).error() } };
	}

	auto execResult = mutation_.exec(::hg::move(mutationFn_));

	mutation_.release(context);
	// Warning: query_.cleanup() should be auto called after guard release.

	return Expected { ::hg::move(execResult) };
}

Result<void, IoQueryGuard<void>::error_type> ExtendedStorageSystem::execQuery(
	ref<Query> query_,
	mref<std::function<Query::exec_noreturn_type(cref<IoResourceAccessorBase>)>> queryFn_
) {

	auto& context = _context;

	/**/

	// Note: Readers share the primary's stack lock until release; commits and removals take it exclusively.
	// shortcut: a storage call nested in another storage callback on the same primary re-takes this FiberSharedMutex on the same fiber and deadlocks once a writer waits (immediately for a mutation nested in a query), upgrade to a per-fiber owner check if nested calls appear.
	::hg::assertrt(query_.primary() != nullptr);
	const auto stackLock = concurrent::SharedLock { context.stacks.lockOf(query_.primary()) };
	if (query_.primary()->isRetired()) {
		return Unexpected { IoQueryGuard<void>::error_type { retiredError() } };
	}

	// Warning: Should have been called already while `fetching` query_.prepare()
	auto acqResult = query_.acquire(context);
	if (not acqResult.has_value()) {
		return Unexpected { IoQueryGuard<void>::error_type { ::hg::move(acqResult).error() } };
	}
	const auto release = ReleaseOnExit { .action = query_, .context = context };

	/* auto execResult = */
	query_.exec(::hg::move(queryFn_));
	// Warning: query_.cleanup() should be auto called after guard release.

	return {};
}

Result<Query::exec_valreturn_type, IoQueryGuard<void>::error_type> ExtendedStorageSystem::execQuery(
	ref<Query> query_,
	mref<std::function<Query::exec_valreturn_type(cref<IoResourceAccessorBase>)>> queryFn_
) {

	auto& context = _context;

	/**/

	// Note: Readers share the primary's stack lock until release; commits and removals take it exclusively.
	// shortcut: a storage call nested in another storage callback on the same primary re-takes this FiberSharedMutex on the same fiber and deadlocks once a writer waits (immediately for a mutation nested in a query), upgrade to a per-fiber owner check if nested calls appear.
	::hg::assertrt(query_.primary() != nullptr);
	const auto stackLock = concurrent::SharedLock { context.stacks.lockOf(query_.primary()) };
	if (query_.primary()->isRetired()) {
		return Unexpected { IoQueryGuard<void>::error_type { retiredError() } };
	}

	// Warning: Should have been called already while `fetching` query_.prepare()
	auto acqResult = query_.acquire(context);
	if (not acqResult.has_value()) {
		return Unexpected { IoQueryGuard<void>::error_type { ::hg::move(acqResult).error() } };
	}
	const auto release = ReleaseOnExit { .action = query_, .context = context };

	auto execResult = query_.exec(::hg::move(queryFn_));
	// Warning: query_.cleanup() should be auto called after guard release.

	return Expected(::hg::move(execResult));
}
