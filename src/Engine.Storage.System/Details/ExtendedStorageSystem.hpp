#pragma once

#include "../StorageSystem.hpp"

namespace hg::engine::resource {
	class ReadWritePackage;
}

namespace hg::engine::storage {
	template <typename>
	class IoMutationGuard;

	template <typename>
	class IoQueryGuard;
}

namespace hg::engine::storage {
	class ExtendedStorageSystem final :
		public StorageSystem {
	public:
		template <typename>
		friend class IoMutationGuard;

		template <typename>
		friend class IoQueryGuard;

	public:
		using this_type = StorageSystem;

		using any_mutation_guard = IoMutationGuard<void>;
		using any_query_guard = IoQueryGuard<void>;

	protected:
		[[nodiscard]] Result<
			void,
			any_mutation_guard::error_type
		> execMutation(
			_Inout_ ref<Mutation> query_,
			_Inout_ mref<std::function<void(cref<IoResourceAccessorBase>)>> mutationFn_
		);

		[[nodiscard]] Result<
			Mutation::exec_valreturn_type,
			any_mutation_guard::error_type
		> execMutation(
			_Inout_ ref<Mutation> mutation_,
			_Inout_ mref<std::function<Mutation::exec_valreturn_type(cref<IoResourceAccessorBase>)>> mutationFn_
		);

	protected:
		[[nodiscard]] Result<
			void,
			any_query_guard::error_type
		> execQuery(
			_Inout_ ref<Query> query_,
			_Inout_ mref<std::function<Query::exec_noreturn_type(cref<IoResourceAccessorBase>)>> queryFn_
		);

		[[nodiscard]] Result<
			Query::exec_valreturn_type,
			any_query_guard::error_type
		> execQuery(
			_Inout_ ref<Query> query_,
			_Inout_ mref<std::function<Query::exec_valreturn_type(cref<IoResourceAccessorBase>)>> queryFn_
		);
	};

	static_assert(sizeof(ExtendedStorageSystem) == sizeof(StorageSystem));
}
