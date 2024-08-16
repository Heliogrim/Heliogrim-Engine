#include "ReflowCommandBuffer.hpp"

#include <algorithm>
#include <ranges>
#include <Engine.Common/Math/Compat.inl>
#include <Engine.GFX/Texture/Texture.hpp>
#include <Engine.GFX/Texture/TextureLikeObject.hpp>
#include <Engine.GFX/Texture/TextureView.hpp>

using namespace hg::engine::reflow;
using namespace hg::engine::gfx;
using namespace hg;

ReflowCommandBuffer::ReflowCommandBuffer() noexcept :
	_scissorStack(),
	_maxLayer(0),
	_layer(0),
	_opaque(),
	_alpha(),
	_scissors(),
	_images(),
	_imageWait(),
	_imageSignal() {}

void ReflowCommandBuffer::begin() {}

void ReflowCommandBuffer::end() {}

void ReflowCommandBuffer::reset(const bool free_) {

	/**
	 *
	 */
	while (not _scissorStack.empty()) {
		_scissorStack.pop();
	}
	_scissors.clear();

	/**
	 *
	 */
	_opaque.clear();
	_alpha.clear();

	_images.clear();
	_imageWait.clear();
	_imageSignal.clear();

	/**
	 *
	 */
	if (free_) {
		_scissors.shrink_to_fit();

		_opaque.shrink_to_fit();
		_alpha.shrink_to_fit();

		_images.shrink_to_fit();
		_imageWait.shrink_to_fit();
		_imageSignal.shrink_to_fit();
	}
}

u32 ReflowCommandBuffer::ensureScissor() {

	const auto& current = _scissorStack.top();

	if (_scissors.empty()) {
		_scissors.emplace_back(current);
		return 0uL;
	}

	if (_scissors.back() == current) {
		return _scissors.size() - 1uL;
	}

	/**/

	_scissors.emplace_back(current);
	return _scissors.size() - 1uL;
}

void ReflowCommandBuffer::pushRootScissor(cref<math::f32ext2d> scissor_) {

	if (not _opaque.indices.empty() || not _alpha.indices.empty()) {

		const u32 lastOpaqueIdx = static_cast<u32>(_opaque.indices.size());
		const u32 lastAlphaIdx = static_cast<u32>(_alpha.indices.size());

		const u32 lastOpaqueMark = _opaque.scissorSpan.empty() ? 0uL : _opaque.scissorSpan.back().last;
		const u32 lastAlphaMark = _alpha.scissorSpan.empty() ? 0uL : _alpha.scissorSpan.back().last;

		u32 scissorIdx = ensureScissor();

		/**/

		_opaque.scissorSpan.emplace_back(lastOpaqueMark, lastOpaqueIdx, scissorIdx);
		_alpha.scissorSpan.emplace_back(lastAlphaMark, lastAlphaIdx, scissorIdx);
	}

	_scissorStack.emplace(scissor_);
}

void ReflowCommandBuffer::pushScissor(cref<math::fExtent2D> scissor_) {

	const u32 lastOpaqueIdx = static_cast<u32>(_opaque.indices.size());
	const u32 lastAlphaIdx = static_cast<u32>(_alpha.indices.size());

	const u32 lastOpaqueMark = _opaque.scissorSpan.empty() ? 0uL : _opaque.scissorSpan.back().last;
	const u32 lastAlphaMark = _alpha.scissorSpan.empty() ? 0uL : _alpha.scissorSpan.back().last;

	const bool touchedOpaqueSpan = lastOpaqueIdx > lastOpaqueMark;
	const bool touchedAlphaSpan = lastAlphaIdx > lastAlphaMark;

	/**/

	u32 scissorIdx = ~0uL;
	if (touchedOpaqueSpan || touchedAlphaSpan) {
		scissorIdx = ensureScissor();
	}

	/**/

	if (touchedOpaqueSpan) {

		// Deduplicate scissor span with same sequential index and consecutive geometry recording
		if (
			not _opaque.scissorSpan.empty() &&
			_opaque.scissorSpan.back().idx == scissorIdx &&
			_opaque.scissorSpan.back().last == lastOpaqueMark
		) {
			_opaque.scissorSpan.back().last = lastOpaqueIdx;

		} else {
			_opaque.scissorSpan.emplace_back(lastOpaqueMark, lastOpaqueIdx, scissorIdx);
		}

	}

	if (touchedAlphaSpan) {

		// Deduplicate scissor span with same sequential index and consecutive geometry recording
		if (
			not _alpha.scissorSpan.empty() &&
			_alpha.scissorSpan.back().idx == scissorIdx &&
			_alpha.scissorSpan.back().last == lastAlphaMark
		) {
			_alpha.scissorSpan.back().last = lastAlphaIdx;

		} else {
			_alpha.scissorSpan.emplace_back(lastAlphaMark, lastAlphaIdx, scissorIdx);
		}
	}

	/**/

	_scissorStack.push(scissor_);
}

void ReflowCommandBuffer::pushIntersectScissor(cref<math::fExtent2D> scissor_) {

	const auto hasPrevScissor { not _scissorStack.empty() };
	auto scissor { scissor_ };

	if (hasPrevScissor) {
		const auto& prev { _scissorStack.top() };

		const math::vec2 outerMin { prev.offsetX, prev.offsetY };
		const math::vec2 outerMax { prev.offsetX + prev.width, prev.offsetY + prev.height };

		const math::vec2 innerMin { scissor_.offsetX, scissor_.offsetY };
		const math::vec2 innerMax { scissor_.offsetX + scissor_.width, scissor_.offsetY + scissor_.height };

		const math::vec2 min { math::compMax<float>(outerMin, innerMin) };
		const math::vec2 max { math::compMin<float>(outerMax, innerMax) };

		if (min.x > max.x || min.y > max.y) {
			scissor.width = 0;
			scissor.height = 0;
			scissor.offsetX = 0;
			scissor.offsetY = 0;

		} else {
			scissor.width = max.x - min.x;
			scissor.height = max.y - min.y;
			scissor.offsetX = min.x;
			scissor.offsetY = min.y;
		}
	}

	pushScissor(scissor);
}

math::fExtent2D ReflowCommandBuffer::popScissor() {

	const u32 lastOpaqueIdx = static_cast<u32>(_opaque.indices.size());
	const u32 lastAlphaIdx = static_cast<u32>(_alpha.indices.size());

	const u32 lastOpaqueMark = _opaque.scissorSpan.empty() ? 0uL : _opaque.scissorSpan.back().last;
	const u32 lastAlphaMark = _alpha.scissorSpan.empty() ? 0uL : _alpha.scissorSpan.back().last;

	const bool touchedOpaqueSpan = lastOpaqueIdx > lastOpaqueMark;
	const bool touchedAlphaSpan = lastAlphaIdx > lastAlphaMark;

	/**/

	u32 scissorIdx = ~0uL;
	if (touchedOpaqueSpan || touchedAlphaSpan) {
		scissorIdx = ensureScissor();
	}

	/**/

	if (touchedOpaqueSpan) {

		// Deduplicate scissor span with same sequential index and consecutive geometry recording
		if (
			not _opaque.scissorSpan.empty() &&
			_opaque.scissorSpan.back().idx == scissorIdx &&
			_opaque.scissorSpan.back().last == lastOpaqueMark
		) {
			_opaque.scissorSpan.back().last = lastOpaqueIdx;

		} else {
			_opaque.scissorSpan.emplace_back(lastOpaqueMark, lastOpaqueIdx, scissorIdx);
		}

	}

	if (touchedAlphaSpan) {

		// Deduplicate scissor span with same sequential index and consecutive geometry recording
		if (
			not _alpha.scissorSpan.empty() &&
			_alpha.scissorSpan.back().idx == scissorIdx &&
			_alpha.scissorSpan.back().last == lastAlphaMark
		) {
			_alpha.scissorSpan.back().last = lastAlphaIdx;

		} else {
			_alpha.scissorSpan.emplace_back(lastAlphaMark, lastAlphaIdx, scissorIdx);
		}
	}

	/**/

	const auto top { _scissorStack.top() };
	_scissorStack.pop();

	/* TODO: We may want to track the AABB of the capture geometry within the current scope ?!
	 *	Using this, we may be able to check whether we actually need to submit the current scissor, cause
	 *	anywhere downstream changing the scissors will implicitly require an updated drawing context,
	 *	resulting in additional draw-calls. Furthermore, it may deduplicate the scissor state within the
	 *	submitted set without backtracking, so we expect a more condense submission set of geometry.
	 */

	return top;
}

bool ReflowCommandBuffer::scissorCull(cref<math::vec2> p_, float r_) const noexcept {

	if (_scissorStack.empty()) {
		return false;
	}

	const auto& scissor { _scissorStack.top() };

	if (p_.x + r_ <= scissor.offsetX || p_.x - r_ > scissor.offsetX + scissor.width) {
		return true;
	}

	if (p_.y + r_ <= scissor.offsetY || p_.y - r_ > scissor.offsetY + scissor.height) {
		return true;
	}

	return false;
}

bool ReflowCommandBuffer::scissorCull(cref<math::vec2> off_, cref<math::vec2> size_) const noexcept {

	if (_scissorStack.empty()) {
		return false;
	}

	const auto& scissor { _scissorStack.top() };

	const math::vec2 rectMax { off_ + size_ };
	if (rectMax.x < scissor.offsetX || rectMax.y < scissor.offsetY) {
		return true;
	}

	const math::vec2 scissorMax { scissor.offsetX + scissor.width, scissor.offsetY + scissor.height };
	if (scissorMax.x < off_.x || scissorMax.y < off_.y) {
		return true;
	}

	return false;
}

bool ReflowCommandBuffer::scissorCull(cref<math::fExtent2D> rect_) const noexcept {

	if (_scissorStack.empty()) {
		return false;
	}

	const auto& scissor { _scissorStack.top() };

	const math::vec2 rectMax { rect_.offsetX + rect_.width, rect_.offsetY + rect_.height };
	if (rectMax.x < scissor.offsetX || rectMax.y < scissor.offsetY) {
		return true;
	}

	const math::vec2 scissorMax { scissor.offsetX + scissor.width, scissor.offsetY + scissor.height };
	if (scissorMax.x < rect_.offsetX || scissorMax.y < rect_.offsetY) {
		return true;
	}

	return false;
}

void ReflowCommandBuffer::pushLayer() noexcept {
	_maxLayer = (std::max)(_maxLayer, ++_layer);
}

void ReflowCommandBuffer::pushLayer(const u8 count_) noexcept {
	_maxLayer = (std::max)(_maxLayer, (_layer += count_));
}

void ReflowCommandBuffer::popLayer() noexcept {
	--_layer;
}

void ReflowCommandBuffer::popLayer(const u8 count_) noexcept {
	_layer -= count_;
}

u8 ReflowCommandBuffer::getCurrentMaxLayer() const noexcept {
	return _maxLayer;
}

bool ReflowCommandBuffer::isAnyAlpha(std::span<gfx::uivertex> vertices_) const noexcept {
	return std::ranges::any_of(
		vertices_,
		[](const auto& vertex_) {
			return vertex_.color.a < 255u;
		}
	);
}

u32 ReflowCommandBuffer::encodeLayerAndInstance(const layer_value_type layer_, const instance_value_type instance_) {
	return (static_cast<u32>(layer_) << 16uL | static_cast<u32>(instance_));
}

void ReflowCommandBuffer::drawLine(math::vec2 p0_, math::vec2 p1_, const float strength_) {

	color strokeColor { 255.F, 255.F, 255.F, 255.F };

	const auto hs = strength_ / 2.F;

	const auto mainAxis = p1_ - p0_;
	const auto ortho = math::vec2 { mainAxis.y, -mainAxis.x }.normalize();

	/**/

	auto& capture = strokeColor.a >= 255.F ? _opaque : _alpha;

	/**
	 *
	 */
	constexpr u32 cqi[] { 0uL, 1uL, 2uL, 2uL, 3uL, 0uL };
	capture.indices.reserve(capture.indices.size() + (sizeof(cqi) / sizeof(u32)));

	const u32 baseIdx { static_cast<u32>(capture.vertices.size()) };
	capture.vertices.reserve(baseIdx + 4uL);

	/**
	 *
	 */
	capture.indices.push_back(baseIdx + cqi[0]);
	capture.indices.push_back(baseIdx + cqi[1]);
	capture.indices.push_back(baseIdx + cqi[2]);
	capture.indices.push_back(baseIdx + cqi[3]);
	capture.indices.push_back(baseIdx + cqi[4]);
	capture.indices.push_back(baseIdx + cqi[5]);

	capture.vertices.push_back(
		uivertex {
			math::vec2 { p0_ + ortho * hs }, math::vec4_t<u8>(strokeColor), math::vec3 {}, _layer
		}
	);
	capture.vertices.push_back(
		uivertex {
			math::vec2 { p1_ + ortho * hs }, math::vec4_t<u8>(strokeColor), math::vec3 {}, _layer
		}
	);
	capture.vertices.push_back(
		uivertex {
			math::vec2 { p1_ - ortho * hs }, math::vec4_t<u8>(strokeColor), math::vec3 {}, _layer
		}
	);
	capture.vertices.push_back(
		uivertex {
			math::vec2 { p0_ - ortho * hs }, math::vec4_t<u8>(strokeColor), math::vec3 {}, _layer
		}
	);

}

void ReflowCommandBuffer::drawTriangle(math::vec2 p0_, math::vec2 p1_, math::vec2 p2_, cref<color> color_) {}

void ReflowCommandBuffer::drawTriangleLine(
	math::vec2 p0_,
	math::vec2 p1_,
	math::vec2 p2_,
	cref<color> color_,
	const float strength_
) {}

void ReflowCommandBuffer::drawQuad(math::vec2 p0_, math::vec2 p1_, math::vec2 p2_, math::vec2 p3_, cref<color> color_) {

	if (color_.a <= 0.F) {
		return;
	}

	/**/

	constexpr u32 cqi[] { 0uL, 1uL, 2uL, 2uL, 3uL, 0uL };
	auto& capture = color_.a >= 255.F ? _opaque : _alpha;

	/**/

	const u32 baseVdx = static_cast<u32>(capture.vertices.size());
	const u32 baseIdx = static_cast<u32>(capture.indices.size());

	capture.indices.resize(baseIdx + (sizeof(cqi) / sizeof(u32)));
	capture.vertices.resize(baseVdx + 4uL);

	/**
	 *
	 */
	capture.indices[baseIdx + 0uL] = (baseVdx + cqi[0]);
	capture.indices[baseIdx + 1uL] = (baseVdx + cqi[1]);
	capture.indices[baseIdx + 2uL] = (baseVdx + cqi[2]);
	capture.indices[baseIdx + 3uL] = (baseVdx + cqi[3]);
	capture.indices[baseIdx + 4uL] = (baseVdx + cqi[4]);
	capture.indices[baseIdx + 5uL] = (baseVdx + cqi[5]);

	capture.vertices[baseVdx + 0uL] = uivertex {
		math::vec2 { p0_ },
		math::vec4_t<u8> {
			static_cast<u8>(color_.r),
			static_cast<u8>(color_.g),
			static_cast<u8>(color_.b),
			static_cast<u8>(color_.a)
		},
		math::vec3 {},
		_layer
	};
	capture.vertices[baseVdx + 1uL] = uivertex {
		math::vec2 { p1_ },
		math::vec4_t<u8> {
			static_cast<u8>(color_.r),
			static_cast<u8>(color_.g),
			static_cast<u8>(color_.b),
			static_cast<u8>(color_.a)
		},
		math::vec3 {},
		_layer
	};
	capture.vertices[baseVdx + 2uL] = uivertex {
		math::vec2 { p2_ },
		math::vec4_t<u8> {
			static_cast<u8>(color_.r),
			static_cast<u8>(color_.g),
			static_cast<u8>(color_.b),
			static_cast<u8>(color_.a)
		},
		math::vec3 {},
		_layer
	};
	capture.vertices[baseVdx + 3uL] = uivertex {
		math::vec2 { p3_ },
		math::vec4_t<u8> {
			static_cast<u8>(color_.r),
			static_cast<u8>(color_.g),
			static_cast<u8>(color_.b),
			static_cast<u8>(color_.a)
		},
		math::vec3 {},
		_layer
	};
}

void ReflowCommandBuffer::drawQuadLine(
	math::vec2 p0_,
	math::vec2 p1_,
	math::vec2 p2_,
	math::vec2 p3_,
	cref<color> color_,
	const float strength_
) {}

void ReflowCommandBuffer::drawRect(math::vec2 min_, math::vec2 max_, cref<color> color_) {}

void ReflowCommandBuffer::drawRectLine(math::vec2 min_, math::vec2 max_, cref<color> color_, const float strength_) {}

void ReflowCommandBuffer::drawArc(
	math::vec2 pos_,
	const float radius_,
	const float fromTheta_,
	const float toTheta_,
	cref<color> color_
) {

	if (color_.a <= 0.F) {
		return;
	}

	/**/

	/* Surface :: 2*PI*r */
	/* Radians :: 2*PI */
	/* Splice  :: (tt - ft) */
	/* Result  :: [2*PI*r] * [(tt-ft)/(2*PI)] */
	/*         :: [r] * [(tt-ft)] */

	u32 segments { MIN(static_cast<u32>(radius_ * (toTheta_ - fromTheta_)), 11uL) };

	// Enforce odd segment count to prevent rounding / flattening of corner tip
	if (not segments & 0x1) {
		++segments;
	}

	const float dtPs { (toTheta_ - fromTheta_) / static_cast<float>(segments) };

	const u32 reqVtx { 1uL + segments + 1uL };
	const u32 reqIdx { segments * 3uL };

	auto& capture = color_.a >= 255.F ? _opaque : _alpha;

	/**
	 *
	 */
	const u32 baseIdx { static_cast<u32>(capture.indices.size()) };
	const u32 baseVtx { static_cast<u32>(capture.vertices.size()) };

	capture.indices.reserve(baseIdx + reqIdx);
	capture.vertices.reserve(baseVtx + reqVtx);

	/**/
	capture.vertices.push_back(
		uivertex {
			math::vec2 { pos_ },
			math::vec4_t<u8> {
				static_cast<u8>(color_.r),
				static_cast<u8>(color_.g),
				static_cast<u8>(color_.b),
				static_cast<u8>(color_.a)
			},
			math::vec3 {},
			_layer
		}
	);

	/**/
	for (u32 seg { 0uL }; seg <= segments; ++seg) {
		float theta { fromTheta_ + static_cast<float>(seg) * dtPs };
		theta = MIN(theta, toTheta_);

		const math::vec2 nd { math::cosf(theta), math::sinf(theta) };

		capture.vertices.push_back(
			uivertex {
				math::vec2 { pos_ + nd * radius_ },
				math::vec4_t<u8> {
					static_cast<u8>(color_.r),
					static_cast<u8>(color_.g),
					static_cast<u8>(color_.b),
					static_cast<u8>(color_.a)
				},
				math::vec3 {},
				_layer
			}
		);
	}

	/**/
	for (u32 seg { 0uL }; seg < segments; ++seg) {
		capture.indices.push_back(baseVtx);
		capture.indices.push_back(baseVtx + seg + 1uL);
		capture.indices.push_back(baseVtx + seg + 2uL);
	}
}

void ReflowCommandBuffer::drawText(
	math::vec2 pos_,
	cref<string_view> text_,
	ref<Font> font_,
	const float fontSize_,
	cref<color> color_
) {

	if (color_.a <= 0.F || text_.empty()) {
		return;
	}

	/**/

	// Warning: Due to the usage of bitmap fonts, we are restricted to use alpha blending
	// auto& capture = color_.a >= 255.F ? _opaque : _alpha;
	auto& capture = _alpha;

	const auto fss { font_.nextFontSize(static_cast<u32>(fontSize_)) };// Next (native) Supported Size
	math::vec2 charScale { fontSize_ / static_cast<float>(fss) };

	/**
	 *
	 */
	const u32 baseIdx { static_cast<u32>(capture.indices.size()) };
	const u32 baseVtx { static_cast<u32>(capture.vertices.size()) };

	const u32 reqIdx { static_cast<u32>(text_.size()) * 6uL };
	const u32 reqVtx { static_cast<u32>(text_.size()) * 4uL };

	//_runningIndexes.reserve(baseIdx + reqIdx);
	//_runningVertices.reserve(baseVtx + reqVtx);

	capture.indices.resize(baseIdx + reqIdx);
	capture.vertices.resize(baseVtx + reqVtx);

	/**
	 *
	 */
	constexpr u32 cqi[] { 0uL, 1uL, 2uL, 2uL, 3uL, 0uL };

	math::vec2 fwd { pos_ + math::vec2 { 0.F, -0.25F * static_cast<float>(fss) * charScale.y } };
	fwd.x = math::floorf(fwd.x);
	fwd.y = math::floorf(fwd.y);

	/**/

	u32 cii = baseIdx;
	u32 cvi = baseVtx;

	for (const auto& letter : text_) {

		/**/
		const auto& glyph { font_.glyph(static_cast<u32>(letter), fss) };

		// TODO: Cleanup <=> Command Invocation should not be required to validate draw command
		if (glyph == nullptr) {
			continue;
		}

		/**/

		capture.indices[cii + 0] = (cvi + cqi[0]);
		capture.indices[cii + 1] = (cvi + cqi[1]);
		capture.indices[cii + 2] = (cvi + cqi[2]);
		capture.indices[cii + 3] = (cvi + cqi[3]);
		capture.indices[cii + 4] = (cvi + cqi[4]);
		capture.indices[cii + 5] = (cvi + cqi[5]);

		cii += 6uL;

		/**/

		const float gx { fwd.x + (static_cast<float>(glyph->_bearing.x) * charScale.x) };
		const float gy { fwd.y + fontSize_ - (static_cast<float>(glyph->_bearing.y) * charScale.y) };
		const float gw { static_cast<float>(glyph->_size.x) * charScale.x };
		const float gh { static_cast<float>(glyph->_size.y) * charScale.y };

		// TODO: Resize vertices and indices, than switch from constructor to assignment

		capture.vertices[cvi + 0] = uivertex {
			math::vec2 {
				gx,
				gy
			},
			math::vec4_t<u8> {
				static_cast<u8>(color_.r),
				static_cast<u8>(color_.g),
				static_cast<u8>(color_.b),
				static_cast<u8>(color_.a)
			},
			math::vec3 { glyph->_minSt, 0.F },
			_layer
		};
		capture.vertices[cvi + 1] = uivertex {
			math::vec2 {
				gx + gw,
				gy
			},
			math::vec4_t<u8> {
				static_cast<u8>(color_.r),
				static_cast<u8>(color_.g),
				static_cast<u8>(color_.b),
				static_cast<u8>(color_.a)
			},
			math::vec3 { glyph->_maxSt.s, glyph->_minSt.t, 0.F },
			_layer
		};
		capture.vertices[cvi + 2] = uivertex {
			math::vec2 {
				gx + gw,
				gy + gh
			},
			math::vec4_t<u8> {
				static_cast<u8>(color_.r),
				static_cast<u8>(color_.g),
				static_cast<u8>(color_.b),
				static_cast<u8>(color_.a)
			},
			math::vec3 { glyph->_maxSt, 0.F },
			_layer
		};
		capture.vertices[cvi + 3] = uivertex {
			math::vec2 {
				gx,
				gy + gh
			},
			math::vec4_t<u8> {
				static_cast<u8>(color_.r),
				static_cast<u8>(color_.g),
				static_cast<u8>(color_.b),
				static_cast<u8>(color_.a)
			},
			math::vec3 { glyph->_minSt.s, glyph->_maxSt.t, 0.F },
			_layer
		};

		cvi += 4uL;

		/**/

		fwd.x += glyph->_advance * charScale.x;
	}

	/**/

	capture.indices.erase(capture.indices.end() - ((baseIdx + reqIdx) - cii), capture.indices.end());
	capture.vertices.erase(capture.vertices.end() - ((baseVtx + reqVtx) - cvi), capture.vertices.end());

	/**/

	_images.emplace_back(font_.atlas().get());
	capture.imageSpan.emplace_back(baseIdx, cii, _images.size() - 1uL);
}

void ReflowCommandBuffer::drawImage(
	math::vec2 p0_,
	math::vec2 uv0_,
	math::vec2 p1_,
	math::vec2 uv1_,
	math::vec2 p2_,
	math::vec2 uv2_,
	math::vec2 p3_,
	math::vec2 uv3_,
	nmpt<gfx::TextureLikeObject> image_,
	ImageDrawOptions options_
) {
	drawImageAsync(
		p0_,
		uv0_,
		p1_,
		uv1_,
		p2_,
		uv2_,
		p3_,
		uv3_,
		std::move(image_),
		nullptr,
		nullptr,
		std::move(options_)
	);
}

void ReflowCommandBuffer::drawImageAsync(
	math::vec2 p0_,
	math::vec2 uv0_,
	math::vec2 p1_,
	math::vec2 uv1_,
	math::vec2 p2_,
	math::vec2 uv2_,
	math::vec2 p3_,
	math::vec2 uv3_,
	nmpt<gfx::TextureLikeObject> image_,
	_::VkSemaphore wait_,
	_::VkSemaphore signal_,
	ImageDrawOptions options_
) {

	/**/
	constexpr u32 cqi[] { 0uL, 1uL, 2uL, 2uL, 3uL, 0uL };
	const auto handleAlpha = (options_.alpha == ReflowAlphaMode::eEnable) || (options_.tint.a < 255u);
	auto& capture = handleAlpha ? _opaque : _alpha;

	/**/

	const u32 baseVdx = static_cast<u32>(capture.vertices.size());
	const u32 baseIdx = static_cast<u32>(capture.indices.size());

	capture.indices.resize(baseIdx + (sizeof(cqi) / sizeof(u32)));
	capture.vertices.resize(baseVdx + 4uL);

	/**
	 *
	 */
	capture.indices[baseIdx + 0uL] = (baseVdx + cqi[0]);
	capture.indices[baseIdx + 1uL] = (baseVdx + cqi[1]);
	capture.indices[baseIdx + 2uL] = (baseVdx + cqi[2]);
	capture.indices[baseIdx + 3uL] = (baseVdx + cqi[3]);
	capture.indices[baseIdx + 4uL] = (baseVdx + cqi[4]);
	capture.indices[baseIdx + 5uL] = (baseVdx + cqi[5]);

	capture.vertices[baseVdx + 0uL] = uivertex {
		math::vec2 { p0_ },
		math::vec4_t<u8> {
			static_cast<u8>(options_.tint.r),
			static_cast<u8>(options_.tint.g),
			static_cast<u8>(options_.tint.b),
			static_cast<u8>(options_.tint.a)
		},
		math::vec3 { uv0_, 0.F },
		_layer
	};
	capture.vertices[baseVdx + 1uL] = uivertex {
		math::vec2 { p1_ },
		math::vec4_t<u8> {
			static_cast<u8>(options_.tint.r),
			static_cast<u8>(options_.tint.g),
			static_cast<u8>(options_.tint.b),
			static_cast<u8>(options_.tint.a)
		},
		math::vec3 { uv1_, 0.F },
		_layer
	};
	capture.vertices[baseVdx + 2uL] = uivertex {
		math::vec2 { p2_ },
		math::vec4_t<u8> {
			static_cast<u8>(options_.tint.r),
			static_cast<u8>(options_.tint.g),
			static_cast<u8>(options_.tint.b),
			static_cast<u8>(options_.tint.a)
		},
		math::vec3 { uv2_, 0.F },
		_layer
	};
	capture.vertices[baseVdx + 3uL] = uivertex {
		math::vec2 { p3_ },
		math::vec4_t<u8> {
			static_cast<u8>(options_.tint.r),
			static_cast<u8>(options_.tint.g),
			static_cast<u8>(options_.tint.b),
			static_cast<u8>(options_.tint.a)
		},
		math::vec3 { uv3_, 0.F },
		_layer
	};

	/**/

	const auto lastImage = [](const auto& capture_, const auto& images_) -> nmpt<TextureLikeObject> {

		if (capture_.imageSpan.empty()) {
			return images_.empty() ? nullptr : images_.back();
		}

		return images_[capture_.imageSpan.back().idx];
	}(capture, _images);

	/**/

	if (lastImage == image_) {
		capture.imageSpan.back().last = baseIdx + 6uL;

	} else {
		_images.emplace_back(std::move(image_));
		capture.imageSpan.emplace_back(baseIdx, baseIdx + 6uL, _images.size() - 1uL);
	}

	/**/

	if (wait_) {
		_imageWait.push_back(wait_);
	}

	if (signal_) {
		_imageSignal.push_back(signal_);
	}
}
