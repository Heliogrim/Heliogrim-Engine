#pragma once

#include <Engine.Common/__macro.hpp>

#include "ActionTypeId.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Reflect;
import Engine.Scheduler;
#endif

namespace hg {
	// Local alias instead of Heliogrim/Async/Waitable.hpp, which does not compile in the generated global module fragment.
	using Signal = engine::scheduler::Signal;
}

namespace hg::editor {
	class macro_novtable Action :
		public InheritBase<Action>,
		public ArcFromThis<Action> {
	public:
		using this_type = Action;

	protected:
		Action() noexcept;

	public:
		constexpr ~Action() noexcept override = default;

	public:
		[[nodiscard]] action_type_id getTypeId() const noexcept;

	public:
		[[nodiscard]] virtual bool isReversible() const noexcept = 0;

		[[nodiscard]] virtual bool isPending() const noexcept = 0;

		[[nodiscard]] virtual bool isRunning() const noexcept = 0;

		[[nodiscard]] virtual bool isFinished() const noexcept = 0;

	public:
		virtual Result<void, std::runtime_error> apply() = 0;

		virtual Result<void, std::runtime_error> revoke() = 0;

		virtual Result<void, std::runtime_error> undo() = 0;

	public:
		[[nodiscard]] virtual ref<Signal> finished() const noexcept = 0;
	};
}// namespace hg::editor
