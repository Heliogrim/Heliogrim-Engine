#pragma once

#include "../ReflowPhase.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
#endif

namespace hg::engine::reflow {
	class Widget;

	struct CellReader {
		wptr<Widget> widget;
		Flag<ReflowPhase, u8> phases;
	};

	/**
	 * Untyped half of a cell: the provider binding and the reader records.
	 */
	class CellBase {
	protected:
		CellBase() noexcept = default;

	public:
		~CellBase() = default;

	private:
		nmpt<Widget> _provider;
		type_id _contextType;
		Vector<CellReader> _readers;

	public:
		/* Called by the owning provider; readers are kept while their cached provider for the context type is this one. */
		void bind(nmpt<Widget> provider_, const type_id contextType_) noexcept;

		[[nodiscard]] nmpt<Widget> provider() const noexcept;

		/* For the inspector. */
		[[nodiscard]] std::span<const CellReader> readers() const noexcept;

	protected:
		void track(ref<Widget> reader_, const Flag<ReflowPhase, u8> phases_);

		void notify();
	};

	/**
	 * Context field; a read subscribes the reader to this field only.
	 */
	template <class T_>
	class Cell :
		public CellBase {
	public:
		using this_type = Cell<T_>;

	public:
		Cell() = default;

		explicit Cell(T_ value_) :
			_value(std::move(value_)) {}

	private:
		T_ _value {};

	public:
		/* Records the reader with its phases; not from a constructor, the reader must be owned by a shared pointer. */
		[[nodiscard]] ref<const T_> read(ref<Widget> reader_, const Flag<ReflowPhase, u8> phases_) {
			track(reader_, phases_);
			return _value;
		}

		[[nodiscard]] ref<const T_> peek() const noexcept {
			return _value;
		}

		/* Marks the live readers when the value changed; returns whether it did. */
		bool set(T_ value_) {
			if (_value == value_) {
				return false;
			}
			_value = std::move(value_);
			notify();
			return true;
		}
	};
}
