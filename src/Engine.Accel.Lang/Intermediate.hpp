#pragma once

#include <Engine.Common/Sal.hpp>

#include "Language.hpp"
#include "Representation.hpp"

namespace hg::engine::accel::lang {
	class Intermediate {
	public:
		using this_type = Intermediate;

	public:
		Intermediate() noexcept = default;

		explicit Intermediate(cref<this_type> other_);

		Intermediate(mref<this_type> other_) noexcept;

		~Intermediate() = default;

	public:
		Language lang;
		Representation rep;

	public:
		[[deprecated]] void applyMask(u8 mask_ = 0xFFu);

		void enumerateOutboundSymbols(
			_Out_ ref<Vector<nmpt<const Symbol>>> outbound_,
			u8 mask_ = 0xFFu
		) const noexcept;

		void enumerateInboundSymbols(
			_Out_ ref<Vector<nmpt<const Symbol>>> inbound_,
			u8 mask_ = 0xFFu
		) const noexcept;
	};
}
