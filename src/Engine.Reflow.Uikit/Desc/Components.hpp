#pragma once

#include "Desc.hpp"
#include "Surface.hpp"
#include "../Atom/Text.hpp"
#include "../Atom/Layout/VerticalLayout.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Common;
import Engine.Reflow;
import std;
#endif

/**
 * Description aggregates. Each engaged member becomes one `set` call at mount; `ref` is filled with the built widget.
 * The `ref` member shadows the `ref<>` alias, so nothing below it in a struct may use that alias.
 */
namespace hg::engine::reflow::uikit::desc {
	template <class Self_>
	struct Describes {
		template <class... Children_> requires (std::convertible_to<Children_, Desc> && ...)
		[[nodiscard]] Desc operator()(Children_&&... children_) const {
			Vector<Desc> children {};
			children.reserve(sizeof...(Children_));
			(children.emplace_back(static_cast<Desc>(std::forward<Children_>(children_))), ...);
			return static_cast<const Self_&>(*this).build(::hg::move(children));
		}

		[[nodiscard]] operator Desc() const {
			return static_cast<const Self_&>(*this).build({});
		}
	};

	struct Column : Describes<Column> {
		Opt<ReflowUnit> minWidth {};
		Opt<ReflowUnit> minHeight {};
		Opt<Padding> padding {};
		Opt<float> colGap {};
		Opt<float> rowGap {};
		String name {};
		ptr<sptr<VerticalLayout>> ref = nullptr;

		[[nodiscard]] Desc build(mref<Vector<Desc>> children_) const;
	};

	struct Label : Describes<Label> {
		Opt<ReflowUnit> minWidth {};
		Opt<ReflowUnit> minHeight {};
		Opt<Padding> padding {};
		Opt<String> text {};
		String name {};
		ptr<sptr<Text>> ref = nullptr;

		[[nodiscard]] Desc build(mref<Vector<Desc>> children_) const;
	};

	struct Surface : Describes<Surface> {
		Opt<ReflowUnit> minWidth {};
		Opt<ReflowUnit> minHeight {};
		Opt<Padding> padding {};
		Opt<color> tint {};
		Opt<math::fvec4> borderRadius {};
		String name {};
		ptr<sptr<uikit::Surface>> ref = nullptr;
		String component {};

		[[nodiscard]] Desc build(mref<Vector<Desc>> children_) const;
	};
}
