#pragma once
#include <Editor.Action/Action/SubAction.hpp>
#include <Engine.Reflect/Inherit/InheritMeta.hpp>

namespace hg::editor {
	class InitializeBuildSystemSubAction final :
		public InheritMeta<InitializeBuildSystemSubAction, SubAction> {
	public:
		using this_type = InitializeBuildSystemSubAction;

	public:
		explicit InitializeBuildSystemSubAction(_In_ cref<Action> parentAction_);

		~InitializeBuildSystemSubAction() override = default;

	public:
		[[nodiscard]] bool isReversible() const noexcept override;

		[[nodiscard]] bool isPending() const noexcept override;

		[[nodiscard]] bool isRunning() const noexcept override;

		[[nodiscard]] bool isFinished() const noexcept override;

	public:
		void apply() override;

		void reverse() override;

	public:
		[[nodiscard]] operator ptr<await_signal_sub_type>() const noexcept override;

		[[nodiscard]] bool failed() const noexcept override;
	};
}
