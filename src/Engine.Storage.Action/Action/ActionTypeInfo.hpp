#pragma once

#include <Engine.Asserts/Asserts.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Reflect/FullTypeInfo.hpp>

#include "StageTypeInfo.hpp"

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
