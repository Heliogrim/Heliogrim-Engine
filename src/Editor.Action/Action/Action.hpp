#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Heliogrim/Async/AwaitSignal.hpp>

#include "ActionTypeId.hpp"

namespace hg::editor {
	class macro_novtable Action {
	public:
		using this_type = Action;

	protected:
		Action(cref<action_type_id> typeId_);

	public:
		virtual ~Action() noexcept = default;

	private:
		action_type_id _typeId;

	public:
		[[nodiscard]] const action_type_id getTypeId() const noexcept;

	public:
		[[nodiscard]] virtual bool isReversible() const noexcept = 0;

		[[nodiscard]] virtual bool isPending() const noexcept = 0;

		[[nodiscard]] virtual bool isRunning() const noexcept = 0;

		[[nodiscard]] virtual bool isFinished() const noexcept = 0;

	public:
		virtual void apply() = 0;

		virtual void reverse() = 0;

	public:
		[[nodiscard]] virtual operator ptr<await_signal_sub_type>() const noexcept = 0;

	public:
		[[nodiscard]] virtual bool failed() const noexcept = 0;
	};
}
