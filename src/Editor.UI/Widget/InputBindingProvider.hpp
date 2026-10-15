#pragma once

#include <Editor.UI.Service/Input/InputBindingList.hpp>
#include <Engine.Reflow/Widget/CompoundWidget.hpp>

namespace hg::editor::ui {
	class InputBindingProvider :
		public engine::reflow::CompoundWidget {
	public:
		using this_type = InputBindingProvider;

	public:
		InputBindingProvider();

		explicit InputBindingProvider(mref<SharedPtr<Widget>> content_);

		~InputBindingProvider() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	private:
		String _input_binding_key;
		Opt<ref<const service::InputBindingList>> _input_bindings;

	public:
		void setContent(mref<SharedPtr<Widget>> content_);

	public:
		engine::reflow::EventResponse invokeOnBlur(cref<engine::reflow::FocusEvent> event_) override;

		engine::reflow::EventResponse invokeOnFocus(cref<engine::reflow::FocusEvent> event_) override;

		engine::reflow::EventResponse invokeOnKeyDown(ref<const engine::reflow::KeyboardEvent> event_) override;
	};
}
