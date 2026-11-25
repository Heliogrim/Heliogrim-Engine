#pragma once

#include "Cell.hpp"
#include "ContextLookup.hpp"
#include "../Widget/CompoundWidget.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Reflect;
#endif

namespace hg::engine::reflow {
	/**
	 * Provides a `Ctx_` to its content's subtree.
	 *
	 * @details `Ctx_` is a struct of `Cell`s exposing them as `cells()` (a `std::tie`), or a struct of plain values
	 *	declaring `static constexpr bool isStatic = true`, which readers read untracked.
	 */
	template <class Ctx_>
	class ContextProvider :
		public CompoundWidget {
	public:
		using this_type = ContextProvider<Ctx_>;

		static constexpr bool isStatic = [] {
			if constexpr (requires { Ctx_::isStatic; }) {
				return static_cast<bool>(Ctx_::isStatic);
			} else {
				return false;
			}
		}();

	public:
		ContextProvider() :
			CompoundWidget(ReflowClassList {}, nullptr),
			_value() {
			if constexpr (not isStatic) {
				std::apply(
					[this](auto&... cells_) {
						(cells_.bind(this, refl::ctid<Ctx_>()), ...);
					},
					_value.cells()
				);
			}
		}

	private:
		Ctx_ _value;

	public:
		[[nodiscard]] string getTag() const noexcept override {
			return std::format(R"(ContextProvider <{:#x}>)", reinterpret_cast<u64>(this));
		}

		[[nodiscard]] ref<Ctx_> value() noexcept {
			return _value;
		}

		[[nodiscard]] ptr<void> providedContext(const type_id contextType_) noexcept override {
			if (contextType_ == refl::ctid<Ctx_>()) {
				return std::addressof(_value);
			}
			return CompoundWidget::providedContext(contextType_);
		}

		void setContent(const sptr<Widget> content_) {
			replaceSingleChild(*this, _children, content_);
		}
	};
}
