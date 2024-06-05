#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Managed/AtomicRefCountedIntrusive.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>
#include <Heliogrim/Inbuilt.hpp>

#include "ActionTypeId.hpp"

namespace hg::editor {
	class __declspec(novtable) Action :
		public InheritBase<Action>,
		public ArcFromThis<Action> {
	public:
		using this_type = Action;

	protected:
		constexpr Action() noexcept = default;

	public:
		virtual ~Action() noexcept = default;

	public:
		[[nodiscard]] action_type_id getTypeId() const noexcept;

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
