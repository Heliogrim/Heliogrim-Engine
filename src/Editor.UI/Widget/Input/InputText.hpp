#pragma once

#include <Engine.Reflow.Uikit/Atom/Text.hpp>
#include <Engine.Reflow/Widget/Input.hpp>
#include <Engine.Reflow.Uikit/Molecule/Layout/Stack.hpp>

#include "../../Attributes/InputDataAttributes.hpp"
#include "../../Event/InputChangeEvent.hpp"

namespace hg::editor::ui {
	struct InputTextDataAttributes final :
		public engine::reflow::DataAttributes<
			engine::reflow::NamedAttribute<engine::reflow::attr::InputData::limits, engine::reflow::DataAttribute<math::vec2_t<
				String::size_type>>>,
			engine::reflow::NamedAttribute<engine::reflow::attr::InputData::value, engine::reflow::DataAttribute<String>>
		> {
		using DataAttributes::DataAttributes;
	};
}

namespace hg::editor::ui {
	class InputText :
		public engine::reflow::Input<String>,
		public engine::reflow::uikit::Atom<InputTextDataAttributes, void, void> {
	public:
		using this_type = InputText;
		using value_type = String;

	public:
		InputText(
			mref<SharedPtr<engine::reflow::uikit::Stack>> content_,
			mref<SharedPtr<engine::reflow::uikit::Text>> text_
		);

		~InputText() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	public:
		[[nodiscard]] bool shouldTick() const noexcept override;

		void tick() override;

	private:
		SharedPtr<engine::reflow::uikit::Text> _text;
		String _stringified;

	public:
		[[nodiscard]] SharedPtr<engine::reflow::uikit::Stack> getContent() const noexcept;

		[[nodiscard]] value_type value() const noexcept override;

		void setValue(StringView next_);

	public:
		void updateValueAndValidity(bool propagate_, bool emit_) override;

	public:
		[[nodiscard]] listen_handle_type onInputChange(listen_fn_type<engine::reflow::InputChangeEvent> listenFn_);

		engine::reflow::EventResponse invokeOnKeyDown(ref<const engine::reflow::KeyboardEvent> event_) override;

		engine::reflow::EventResponse invokeOnBlur(cref<engine::reflow::FocusEvent> event_) override;

	public:
		void onAttachDataWatcher(ref<engine::reflow::DataWatcher> watcher_) override;

		void onDetachDataWatcher(ref<engine::reflow::DataWatcher> watcher_) override;
	};

	/**/

	[[nodiscard]] extern SharedPtr<InputText> makeInputText();
}
