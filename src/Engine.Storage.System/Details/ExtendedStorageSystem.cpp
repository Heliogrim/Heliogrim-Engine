#include "ExtendedStorageSystem.hpp"

using namespace hg::engine::storage;
using namespace hg;

Result<void, IoMutationGuard<void>::error_type> ExtendedStorageSystem::execMutation(
	ref<Mutation> query_,
	mref<std::function<void(cref<IoResourceAccessorBase>)>> mutationFn_
) {

	auto& context = _context;

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

	/**/

	// Error: unknown file: error: SEH exception with code 0xc0000005 thrown in the test body.
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

	// Warning: Should have been called already while `fetching` query_.prepare()
	auto acqResult = query_.acquire(context);
	if (not acqResult.has_value()) {
		return Unexpected { IoQueryGuard<void>::error_type { ::hg::move(acqResult).error() } };
	}

	/* auto execResult = */
	query_.exec(::hg::move(queryFn_));

	/* auto relResult = */
	query_.release(context);
	// Warning: query_.cleanup() should be auto called after guard release.

	return {};
}

Result<Query::exec_valreturn_type, IoQueryGuard<void>::error_type> ExtendedStorageSystem::execQuery(
	ref<Query> query_,
	mref<std::function<Query::exec_valreturn_type(cref<IoResourceAccessorBase>)>> queryFn_
) {

	auto& context = _context;

	/**/

	// Warning: Should have been called already while `fetching` query_.prepare()
	auto acqResult = query_.acquire(context);
	if (not acqResult.has_value()) {
		return Unexpected { IoQueryGuard<void>::error_type { ::hg::move(acqResult).error() } };
	}

	auto execResult = query_.exec(::hg::move(queryFn_));

	/* auto relResult = */
	query_.release(context);
	// Warning: query_.cleanup() should be auto called after guard release.

	return Expected(::hg::move(execResult));
}
