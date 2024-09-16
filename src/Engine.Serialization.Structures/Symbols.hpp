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
		/* Generic */
		constexpr static Symbol Type {
			.id = "@type"_typeId,
			.str = "@type"sv
		};
		constexpr static Symbol Guid {
			.id = "@guid"_typeId,
			.str = "@guid"sv
		};
		/* Contextual */
		constexpr static Symbol Parent {
			.id = "@parent"_typeId,
			.str = "@parent"sv
		};
		constexpr static Symbol Components {
			.id = "@components"_typeId,
			.str = "@components"sv
		};
	};
}
