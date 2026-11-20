#pragma once

#include <Engine.Common/Sal.hpp>

#include "NativeQueue.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::render::cmd {
	class NativeBatch {
	public:
		using this_type = NativeBatch;

	public:
		constexpr NativeBatch() noexcept = default;

		constexpr virtual ~NativeBatch() noexcept = default;

	public:
		[[nodiscard]] virtual bool ready() const noexcept = 0;

		[[nodiscard]] virtual bool isFaf() const noexcept = 0;

	public:
		virtual void commit() = 0;

		virtual void commitAndDispose() = 0;

	public:
		virtual bool enumerateNativeQueues(
			_Out_ ref<InlineAutoArray<ptr<NativeQueue>, 2uLL>> queues_
		) const noexcept = 0;
	};
}
