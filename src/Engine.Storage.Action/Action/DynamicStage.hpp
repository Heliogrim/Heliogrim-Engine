#pragma once

#include <Engine.Storage.Registry/IStorage.hpp>

#include "../__fwd.hpp"
#include "../Action.hpp"
#include "../Access/IoResourceAccessorBase.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Reflect.Utils;
#endif

namespace hg::engine::storage::system {
	struct DynamicStage {
		Arci<IStorage> storage;
		type_id resourceType;
		/* False for a resource with a read cursor (see `CursorResource`); such a stage is built per query. */
		bool shareable = true;
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
