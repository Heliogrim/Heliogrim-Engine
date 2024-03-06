#pragma once

namespace hg {
	class __declspec(novtable) ResourceBase {
	public:
		constexpr ResourceBase() noexcept = default;

		constexpr virtual ~ResourceBase() noexcept = default;

	public:
		[[nodiscard]] virtual bool isShared() const noexcept = 0;
	};
}
