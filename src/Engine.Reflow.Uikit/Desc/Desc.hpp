#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Reflow;
import std;
#endif

namespace hg::engine::reflow::uikit {
	/**
	 * A description of one widget: how to build it, the descriptions of its children and the name `find` matches.
	 */
	class Desc {
	public:
		Desc(mref<std::function<sptr<Widget>()>> make_, mref<Vector<Desc>> children_, StringView name_);

		Desc(ref<const Desc>) = delete;

		Desc(mref<Desc>) noexcept = default;

		~Desc();

	public:
		ref<Desc> operator=(ref<const Desc>) = delete;

		ref<Desc> operator=(mref<Desc>) noexcept = default;

	public:
		std::function<sptr<Widget>()> make;
		Vector<Desc> children;
		String name;
	};

	/**
	 * Builds the described widget, mounts the children in order and stores the name on the widget.
	 */
	[[nodiscard]] sptr<Widget> mount(mref<Desc> desc_);

	/**
	 * Depth-first search for the first widget named `name_` which is a `T_`.
	 */
	template <class T_>
	[[nodiscard]] sptr<T_> find(ref<const sptr<Widget>> root_, StringView name_) {
		if (root_ == nullptr) {
			return nullptr;
		}

		if (not name_.empty() && root_->getName() == name_) {
			if (auto match = std::dynamic_pointer_cast<T_>(root_); match != nullptr) {
				return match;
			}
		}

		const auto children = root_->children();
		if (children == nullptr) {
			return nullptr;
		}

		for (const auto& child : *children) {
			if (auto match = find<T_>(child, name_); match != nullptr) {
				return match;
			}
		}
		return nullptr;
	}
}
