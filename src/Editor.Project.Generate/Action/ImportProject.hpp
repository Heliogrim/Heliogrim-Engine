#pragma once

#include <Editor.Action/Action/Action.hpp>

namespace hg::editor {
	class ImportProjectAction final :
		public Action {
	public:
		using this_type = ImportProjectAction;

	public:
		ImportProjectAction();

		~ImportProjectAction() override;

	public:
		[[nodiscard]] bool isReversible() const noexcept override;

		[[nodiscard]] bool isPending() const noexcept override;

		[[nodiscard]] bool isRunning() const noexcept override;

		[[nodiscard]] bool isFinished() const noexcept override;

	public:
		void apply() override;

		void reverse() override;

	public:
		[[nodiscard]] explicit operator ptr<await_signal_sub_type>() const noexcept override;

		[[nodiscard]] bool failed() const noexcept override;
	};
}
