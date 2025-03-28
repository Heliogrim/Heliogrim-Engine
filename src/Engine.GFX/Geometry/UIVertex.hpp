#pragma once

#include <Engine.Common/Types.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::gfx {
	/**
	 * A ui-vertex with template attribute types
	 *
	 * @author Julius
	 * @date 19.11.2020
	 *
	 * @tparam pt Type of the position vector.
	 * @tparam ct Type of the color {vector}
	 * @tparam uvmt Type of the uvm mapping {vector}.
	 */
	template <
		typename pt = float,
		typename ct = u8,
		typename uvmt = float,
		typename lt = u8>
	struct uivertex_t {
		using position_type = math::vec2_t<pt>;
		using color_type = math::vec4_t<ct>;
		using uvm_type = math::vec3_t<uvmt>;
		using layer_type = lt;

		constexpr uivertex_t() = default;

		constexpr uivertex_t(
			cref<position_type> position_,
			cref<color_type> color_,
			cref<uvm_type> uvm_,
			const layer_type layer_
		) :
			position(position_),
			color(color_),
			uvm(uvm_),
			layer(layer_) {}

		/** The position */
		position_type position;

		/** The color */
		color_type color;

		/** The uvm */
		uvm_type uvm;

		layer_type layer;
	};

	template <typename pt, typename ct, typename uvmt, typename nt>
	bool operator==(const uivertex_t<pt, ct, uvmt>& left_, const uivertex_t<pt, ct, uvmt>& right_) {
		return left_.pos == right_.pos &&
			left_.color == right_.color &&
			left_.texCoord == right_.texCoord &&
			left_.layer == right_.layer;
	}

	/** Defines an alias representing the vertex */
	typedef uivertex_t<float, u8, float> uivertex;
}
