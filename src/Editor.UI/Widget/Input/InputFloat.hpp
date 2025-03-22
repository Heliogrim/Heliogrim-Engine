#pragma once

#include <Engine.Reflow.Uikit/Atom/Text.hpp>
#include <Engine.Reflow/Widget/Input.hpp>
#include <Engine.Reflow.Uikit/Molecule/Layout/Stack.hpp>

#include "../../Event/InputChangeEvent.hpp"

namespace hg::editor::ui {
	namespace attr {
		struct InputData {
			constexpr static inline auto limits = "limits"sv;
			constexpr static inline auto precision = "precision"sv;
			constexpr static inline auto step = "step"sv;
			constexpr static inline auto value = "value"sv;
		};
	}

	enum class InputFloatPrecision {
		eF64,
		eDouble = eF64,
		eF32,
		eFloat = eF32,
		eF16
	};

	struct InputFloatDataAttributes final :
		public engine::reflow::DataAttributes<
			engine::reflow::NamedAttribute<attr::InputData::limits, engine::reflow::DataAttribute<math::vec2_t<::hg::f64>>>,
			engine::reflow::NamedAttribute<attr::InputData::precision, engine::reflow::DataAttribute<InputFloatPrecision>>,
			engine::reflow::NamedAttribute<attr::InputData::step, engine::reflow::DataAttribute<::hg::f64>>,
			engine::reflow::NamedAttribute<attr::InputData::value, engine::reflow::DataAttribute<::hg::f64>>
		> {
		using DataAttributes::DataAttributes;
	};
}

namespace hg::editor::ui {
	class InputFloat :
		public engine::reflow::Input<f64>,
		public engine::reflow::uikit::Atom<InputFloatDataAttributes, void, void> {
	public:
		using this_type = InputFloat;
		using value_type = f64;

	public:
		InputFloat(
			mref<SharedPtr<engine::reflow::uikit::Stack>> content_,
			mref<SharedPtr<engine::reflow::uikit::Text>> text_
		);

		~InputFloat() override;

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

		void setValue(value_type next_);

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

	[[nodiscard]] extern SharedPtr<InputFloat> makeInputFloat();
}
