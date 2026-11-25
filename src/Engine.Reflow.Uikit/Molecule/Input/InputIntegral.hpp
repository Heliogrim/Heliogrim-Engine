#pragma once

#include "../../Atom/Atom.hpp"
#include "../../Atom/Text.hpp"
#include "../Layout/Stack.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Reflow;
#endif

namespace hg::engine::reflow::uikit {
	enum class InputIntegralPrecision {
		eS64,
		eU64,
		eS32,
		eU32,
		eS16,
		eU16,
		eS8,
		eU8
	};

	struct InputIntegralDataAttributes final :
		public engine::reflow::DataAttributes<
			engine::reflow::NamedAttribute<engine::reflow::attr::InputData::limits, engine::reflow::DataAttribute<math::vec2_t<::hg::s64>>>,
			engine::reflow::NamedAttribute<engine::reflow::attr::InputData::precision, engine::reflow::DataAttribute<
				InputIntegralPrecision>>,
			engine::reflow::NamedAttribute<engine::reflow::attr::InputData::step, engine::reflow::DataAttribute<::hg::s64>>,
			engine::reflow::NamedAttribute<engine::reflow::attr::InputData::value, engine::reflow::DataAttribute<::hg::s64>>
		> {
		using DataAttributes::DataAttributes;
	};
}

namespace hg::engine::reflow::uikit {
	class InputIntegral :
		public engine::reflow::Input<s64>,
		public engine::reflow::uikit::Atom<InputIntegralDataAttributes, void, void> {
	public:
		using this_type = InputIntegral;
		using value_type = s64;

	public:
		InputIntegral(
			mref<SharedPtr<engine::reflow::uikit::Stack>> content_,
			mref<SharedPtr<engine::reflow::uikit::Text>> text_
		);

		~InputIntegral() override;

	public:
		[[nodiscard]] string getTag() const noexcept override;

		[[nodiscard]] bool isFocusable() const noexcept override;

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

		engine::reflow::EventReply invokeOnKeyDown(ref<const engine::reflow::KeyboardEvent> event_) override;

		engine::reflow::EventReply invokeOnBlur(cref<engine::reflow::FocusEvent> event_) override;

	public:
		void onAttachDataWatcher(ref<engine::reflow::DataWatcher> watcher_) override;

		void onDetachDataWatcher(ref<engine::reflow::DataWatcher> watcher_) override;
	};

	/**/

	[[nodiscard]] extern SharedPtr<InputIntegral> makeInputIntegral();
}
