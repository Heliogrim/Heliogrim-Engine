#pragma once

#include <Editor.Action/Action/Action.hpp>
#include <Engine.Filesystem/Url.hpp>

namespace hg::editor {
	class NewProjectAction final :
		public InheritMeta<NewProjectAction, Action> {
	public:
		using this_type = NewProjectAction;

	public:
		NewProjectAction() = delete;

		NewProjectAction(mref<String> projectName_, mref<fs::Url> projectBasePath_);

		~NewProjectAction() override;

	private:
		String _projectName;
		fs::Url _projectBasePath;

	public:
		[[nodiscard]] String getProjectName() const noexcept;

		[[nodiscard]] fs::Url getProjectBasePath() const noexcept;

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
