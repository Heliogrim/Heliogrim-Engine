#pragma once

#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Reflect/FullTypeInfo.hpp>

#include "StageTypeInfo.hpp"
#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Asserts;
#endif

namespace hg::engine::storage {
	struct ActionTypeInfo {
		refl::FullTypeInfo targetResourceInfo {};

		Vector<StageTypeInfo> stageTypeInfos {};

		[[nodiscard]] cref<refl::PartialTypeInfo> baseStorageInfo() const {
			::hg::assertrt(not stageTypeInfos.empty());
			return stageTypeInfos.back().storage;
		}
	};
}
