#pragma once

#include <tuple>

#include "../Input.hpp"
#include "../../Children.hpp"

namespace hg::engine::reflow {
	template <typename... InputTypes_>
	class InputVector :
		public Input<std::tuple<std::decay_t<InputTypes_>...>> {
	public:
		using this_type = InputVector<InputTypes_...>;
		using base_type = Input<std::tuple<std::decay_t<typename InputTypes_::value_type>...>>;

		using base_type::input_type;

	public:
		InputVector() :
			base_type() {
			setup();
		}

		~InputVector() override = default;

	public:
		[[nodiscard]] string getTag() const noexcept override {
			return std::format(R"(InputVector <{:#x}>)", reinterpret_cast<u64>(this));
		}

	private:
		Vector<sptr<InputBase>> _inputs;

	private:
		void setup() {
			// TODO: ...
		}

	private:
		SharedPtr<Widget> _content;

	public:
		void setContent(ref<SharedPtr<Widget>> content_) {
			_content = content_;
		}

	public:
		[[nodiscard]] const ptr<const Children> children() const override {
			return _content->children();
		}

	public:
		void enable() override {

			const auto& children { *_content->children() };
			for (size_t i { 1uLL }; i < children.size(); i += 2uLL) {
				const auto& child { children[i] };
				std::static_pointer_cast<InputBase, Widget>(child)->enable();
			}
		}

		void disable() override {

			const auto& children { *_content->children() };
			for (size_t i { 1uLL }; i < children.size(); i += 2uLL) {
				const auto& child { children[i] };
				std::static_pointer_cast<InputBase, Widget>(child)->disable();
			}
		}

		void markAsDirty() override {

			const auto& children { *_content->children() };
			for (size_t i { 1uLL }; i < children.size(); i += 2uLL) {
				const auto& child { children[i] };
				std::static_pointer_cast<InputBase, Widget>(child)->markAsDirty();
			}
		}

		void markAsPristine() override {

			const auto& children { *_content->children() };
			for (size_t i { 1uLL }; i < children.size(); i += 2uLL) {
				const auto& child { children[i] };
				std::static_pointer_cast<InputBase, Widget>(child)->markAsPristine();
			}
		}

		void markAsTouched() override {
			const auto& children { *_content->children() };
			for (size_t i { 1uLL }; i < children.size(); i += 2uLL) {
				const auto& child { children[i] };
				std::static_pointer_cast<InputBase, Widget>(child)->markAsTouched();
			}
		}

		void markAsUntouched() override {

			const auto& children { *_content->children() };
			for (size_t i { 1uLL }; i < children.size(); i += 2uLL) {
				const auto& child { children[i] };
				std::static_pointer_cast<InputBase, Widget>(child)->markAsTouched();
			}
		}

		void reset() override {

			const auto& children { *_content->children() };
			for (size_t i { 1uLL }; i < children.size(); i += 2uLL) {
				const auto& child { children[i] };
				std::static_pointer_cast<InputBase, Widget>(child)->reset();
			}
		}

		void updateValueAndValidity(const bool propagate_, const bool emit_) override {

			const auto& children { *_content->children() };
			for (size_t i { 1uLL }; i < children.size(); i += 2uLL) {
				const auto& child { children[i] };
				std::static_pointer_cast<InputBase, Widget>(child)->updateValueAndValidity(true, emit_);
			}
		}

	public:
		[[nodiscard]] input_type value() const noexcept override {
			return input_type {};
		}
	};
}
