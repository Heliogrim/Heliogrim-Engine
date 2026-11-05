#pragma once

#include <Engine.Common/Guid.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg {
	struct ComponentGuid : public Guid {
		using Guid::Guid;

		constexpr explicit ComponentGuid(ref<const Guid> guid_) noexcept :
			Guid(guid_) {}

	public:
		[[nodiscard]] constexpr bool operator==(ref<const ComponentGuid> other_) const noexcept {
			return Guid::operator==(other_);
		}

		// No comparison through the Guid base: neither against a bare Guid nor against the other guid kind.
		bool operator==(ref<const Guid>) const = delete;

		bool operator!=(ref<const Guid>) const = delete;

		[[nodiscard]] constexpr std::strong_ordering operator<=>(ref<const ComponentGuid> other_) const noexcept {
			return Guid::operator<=>(other_);
		}

		std::strong_ordering operator<=>(ref<const Guid>) const = delete;
	};

	// Guid on the left picks Guid's members through slicing; the inherited operator!= also blocks the reversed member candidate.
	bool operator==(ref<const Guid>, ref<const ComponentGuid>) = delete;

	bool operator!=(ref<const Guid>, ref<const ComponentGuid>) = delete;
}
