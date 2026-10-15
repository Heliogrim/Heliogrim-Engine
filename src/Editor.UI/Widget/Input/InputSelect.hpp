#pragma once

#include "../../Attributes/InputDataAttributes.hpp"
#include "../../Event/InputChangeEvent.hpp"
#include "../../Event/SelectMenuEvent.hpp"
#include "Engine.Reflow.Attributes/Data/DataAttributes.hpp"
#include "Engine.Reflow.Uikit/Atom/Atom.hpp"
#include "Engine.Reflow.Uikit/Atom/Text.hpp"
#include "Engine.Reflow/Widget/Input.hpp"
#include "Engine.Reflow.Uikit/Molecule/Layout/Stack.hpp"

namespace hg::editor::ui {
	struct InputSelectDataAttributes final :
		public engine::reflow::DataAttributes<
			engine::reflow::NamedAttribute<engine::reflow::attr::InputData::value, engine::reflow::DataAttribute<String>>
		> {
		using DataAttributes::DataAttributes;
	};
}

namespace hg::editor::ui {
	class InputSelect :
		public engine::reflow::Input<String>,
		public engine::reflow::uikit::Atom<InputSelectDataAttributes, void, void> {
	public:
		using this_type = InputSelect;
		using value_type = String;

	public:
		InputSelect(
			mref<SharedPtr<engine::reflow::uikit::Stack>> content_,
			mref<SharedPtr<engine::reflow::uikit::Text>> text_
		);

		~InputSelect() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	public:
		[[nodiscard]] bool shouldTick() const noexcept override;

		void tick() override;

	private:
		SharedPtr<engine::reflow::uikit::Text> _text;
		SharedPtr<engine::reflow::Host> _runtimeMenuHost;

	private:
		void openMenu();

		void closeMenu();

	public:
		[[nodiscard]] listen_handle_type onInputChange(listen_fn_type<engine::reflow::InputChangeEvent> listenFn_);

		[[nodiscard]] listen_handle_type onSelectMenu(listen_fn_type<engine::reflow::SelectMenuEvent> listenFn_);

		bool dropOnSelectMenu(listen_handle_type handle_);

		engine::reflow::EventResponse invokeOnSelectMenu(ref<const engine::reflow::SelectMenuEvent> event_) const;

		engine::reflow::EventResponse invokeOnMouseButtonDown(ref<const engine::reflow::MouseEvent> event_) override;

		engine::reflow::EventResponse invokeOnFocus(cref<engine::reflow::FocusEvent> event_) override;

		engine::reflow::EventResponse invokeOnBlur(cref<engine::reflow::FocusEvent> event_) override;

	public:
		void onAttachDataWatcher(ref<engine::reflow::DataWatcher> watcher_) override;

		void onDetachDataWatcher(ref<engine::reflow::DataWatcher> watcher_) override;

	public:
		void updateValueAndValidity(bool propagate_, bool emit_) override;

		[[nodiscard]] input_type value() const noexcept override;
	};

	/**/

	[[nodiscard]] extern SharedPtr<InputSelect> makeInputSelect();
}
