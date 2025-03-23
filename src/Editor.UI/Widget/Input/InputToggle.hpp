#pragma once

#include <Engine.Reflow.Uikit/Atom/Text.hpp>
#include <Engine.Reflow/Widget/Input.hpp>
#include <Engine.Reflow.Uikit/Molecule/Layout/Stack.hpp>

#include "../../Attributes/InputDataAttributes.hpp"
#include "../../Event/InputChangeEvent.hpp"

namespace hg::editor::ui {
	struct InputToggleDataAttributes final :
		public engine::reflow::DataAttributes<
			engine::reflow::NamedAttribute<engine::reflow::attr::InputData::value, engine::reflow::DataAttribute<bool>>
		> {
		using DataAttributes::DataAttributes;
	};
}

namespace hg::editor::ui {
	class InputToggle :
		public engine::reflow::Input<bool>,
		public engine::reflow::uikit::Atom<InputToggleDataAttributes, void, void> {
	public:
		using this_type = InputToggle;
		using value_type = bool;

	public:
		InputToggle();

		~InputToggle() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

	public:
		[[nodiscard]] bool shouldTick() const noexcept override;

		void tick() override;

	private:
		engine::reflow::ReflowEventEmitter::handle_type _activeToggleListener;
		SharedPtr<Widget> _activeContent;
		engine::reflow::ReflowEventEmitter::handle_type _inactiveToggleListener;
		SharedPtr<Widget> _inactiveContent;

	public:
		void setActiveContent(mref<SharedPtr<Widget>> active_);

		void setInactiveContent(mref<SharedPtr<Widget>> inactive_);

		[[nodiscard]] value_type value() const noexcept override;

		void setValue(bool enabled_);

	public:
		void updateValueAndValidity(bool propagate_, bool emit_) override;

	public:
		[[nodiscard]] listen_handle_type onInputChange(listen_fn_type<engine::reflow::InputChangeEvent> listenFn_);

	public:
		void onAttachDataWatcher(ref<engine::reflow::DataWatcher> watcher_) override;

		void onDetachDataWatcher(ref<engine::reflow::DataWatcher> watcher_) override;
	};

	/**/

	[[nodiscard]] extern SharedPtr<InputToggle> makeInputToggle(
		mref<SharedPtr<engine::reflow::Widget>> active_,
		mref<SharedPtr<engine::reflow::Widget>> inactive_,
		bool value_ = false
	);
}
