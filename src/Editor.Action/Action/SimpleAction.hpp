#pragma once

#include "Action.hpp"

#include <atomic>

namespace hg::editor {
	class macro_novtable SimpleAction :
		public InheritMeta<SimpleAction, Action> {
	public:
		using this_type = Action;

	protected:
		constexpr SimpleAction() noexcept = default;

	public:
		~SimpleAction() noexcept override;

	private:
		std::atomic_flag _running;
		mutable Signal _finished;

	protected:
		void setRunning();

		void setFinished();

	public:
		[[nodiscard]] bool isPending() const noexcept override;

		[[nodiscard]] bool isRunning() const noexcept override;

		[[nodiscard]] bool isFinished() const noexcept override;

	public:
		[[nodiscard]] ref<Signal> finished() const noexcept override;
	};
}
