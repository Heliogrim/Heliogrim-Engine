#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Meta/TypeId.hpp>

namespace hg::engine::serialization {
	struct Symbol {
		type_id id;
		StringView str;

		[[nodiscard]] constexpr operator type_id() const noexcept {
			return id;
		}

		[[nodiscard]] constexpr operator StringView() const noexcept {
			return str;
		}
	};

	struct Symbols {
		constexpr static Symbol Type {
			.id = "@type"_typeId,
			.str = "@type"sv
		};
	};
}
