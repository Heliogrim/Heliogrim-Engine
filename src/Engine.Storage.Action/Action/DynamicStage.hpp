#pragma once

#include <functional>
#include <Engine.Common/Expected.hpp>
#include <Engine.Common/Managed/Rc.hpp>
#include <Engine.Reflect.Utils/With/WithType.hpp>
#include <Engine.Storage.Registry/IStorage.hpp>

#include "../Action.hpp"
#include "../__fwd.hpp"
#include "../Access/IoResourceAccessorBase.hpp"

namespace hg::engine::storage::system {
	struct DynamicStage {
		Arci<IStorage> storage;
		std::function<Result<meta::WithType<Rc<IoResourceAccessorBase>>, Action::acq_error_type>(
			ref<Context>,
			cref<Arci<IStorage>>,
			cref<meta::WithType<Rc<IoResourceAccessorBase>>>
		)> acqFn;
		meta::WithType<Rc<IoResourceAccessorBase>> access;
		std::function<void(
			ref<Context>,
			cref<Arci<IStorage>>,
			mref<meta::WithType<Rc<IoResourceAccessorBase>>>
		)> relFn;

	public:
		[[nodiscard]] constexpr bool prepared() const noexcept {
			return storage != nullptr && static_cast<bool>(acqFn) && static_cast<bool>(relFn);
		}

		[[nodiscard]] constexpr bool active() const noexcept {
			return access.value != nullptr;
		}
	};
}
