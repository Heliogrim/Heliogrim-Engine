#pragma once
#include <Editor.Action/Action/SubAction.hpp>
#include <Engine.Reflect/Inherit/InheritMeta.hpp>

namespace hg::editor {
	class LinkEditorResourcesSubAction final :
		public InheritMeta<LinkEditorResourcesSubAction, SubAction> {
	public:
		using this_type = LinkEditorResourcesSubAction;

	public:
		explicit LinkEditorResourcesSubAction(_In_ cref<Action> parentAction_);

		~LinkEditorResourcesSubAction() override = default;

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
