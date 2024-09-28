#pragma once

namespace hg {
	class macro_novtable ResourceBase {
	public:
		constexpr ResourceBase() noexcept = default;

		constexpr virtual ~ResourceBase() noexcept = default;

	public:
		[[nodiscard]] virtual bool isShared() const noexcept = 0;
	};
}
