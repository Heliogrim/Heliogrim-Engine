#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Collection/Stack.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Math/__default.inl>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX/Aabb.hpp>
#include <Engine.GFX/API/__vkFwd.hpp>
#include <Engine.GFX/Color/Color.hpp>
#include <Engine.GFX/Geometry/UIVertex.hpp>

#include "../ReflowAlpha.hpp"
#include "../Font/Font.hpp"

namespace hg::engine::reflow {
	struct ScissorSpan {
		u32 first;
		u32 last;
		u32 idx;
	};

	struct ImageSpan {
		u32 first;
		u32 last;
		u32 idx;
	};

	struct CaptureData {
		Vector<u32> indices;
		Vector<gfx::uivertex> vertices;

		Vector<ScissorSpan> scissorSpan;
		Vector<ImageSpan> imageSpan;

	public:
		void clear() noexcept {
			indices.clear();
			vertices.clear();

			scissorSpan.clear();
			imageSpan.clear();
		}

		void shrink_to_fit() noexcept {
			indices.shrink_to_fit();
			vertices.shrink_to_fit();

			scissorSpan.shrink_to_fit();
			imageSpan.shrink_to_fit();
		}
	};

	struct ImageDrawOptions {
		ReflowAlphaMode alpha = ReflowAlphaMode::eNone;
		color tint;
	};

	class ReflowCommandBuffer {
	public:
		using this_type = ReflowCommandBuffer;

		using layer_value_type = u16;
		using instance_value_type = u16;

	public:
		ReflowCommandBuffer() noexcept;

	public:
		void begin();

		void end();

		void reset(const bool free_ = false);

	public:
		Stack<math::fExtent2D> _scissorStack;

	private:
		[[nodiscard]] u32 ensureScissor();

	public:
		void pushRootScissor(cref<math::f32ext2d> scissor_);

		void pushScissor(cref<math::fExtent2D> scissor_);

		void pushIntersectScissor(cref<math::fExtent2D> scissor_);

		math::fExtent2D popScissor();

	public:
		[[nodiscard]] bool scissorCull(cref<math::vec2> p_, float r_) const noexcept;

		[[nodiscard]] bool scissorCull(cref<math::vec2> off_, cref<math::vec2> size_) const noexcept;

		[[nodiscard]] bool scissorCull(cref<math::fExtent2D> rect_) const noexcept;

		//private:
	public:
		CaptureData _opaque;
		CaptureData _alpha;

		Vector<math::fExtent2D> _scissors;

		Vector<nmpt<gfx::TextureLikeObject>> _images;
		Vector<_::VkSemaphore> _imageWait;
		Vector<_::VkSemaphore> _imageSignal;

	private:
		[[nodiscard]] bool isAnyAlpha(std::span<gfx::uivertex> vertices_) const noexcept;

		[[nodiscard]] u32 encodeLayerAndInstance(const layer_value_type layer_, const instance_value_type instance_);

	public:
		[[nodiscard]] constexpr static u32 maxRenderLayers() noexcept {
			return (std::numeric_limits<layer_value_type>::max)();
		}

		[[nodiscard]] constexpr static u32 maxInstancesPerLayer() noexcept {
			return (std::numeric_limits<instance_value_type>::max)();
		}

	public:
		void drawLine(math::vec2 p0_, math::vec2 p1_, const float strength_);

		void drawTriangle(math::vec2 p0_, math::vec2 p1_, math::vec2 p2_, cref<color> color_);

		void drawTriangleLine(
			math::vec2 p0_,
			math::vec2 p1_,
			math::vec2 p2_,
			cref<color> color_,
			const float strength_
		);

		void drawQuad(math::vec2 p0_, math::vec2 p1_, math::vec2 p2_, math::vec2 p3_, cref<color> color_);

		void drawQuadLine(
			math::vec2 p0_,
			math::vec2 p1_,
			math::vec2 p2_,
			math::vec2 p3_,
			cref<color> color_,
			const float strength_
		);

		void drawRect(math::vec2 min_, math::vec2 max_, cref<color> color_);

		void drawRectLine(math::vec2 min_, math::vec2 max_, cref<color> color_, const float strength_);

		void drawArc(
			math::vec2 pos_,
			const float radius_,
			const float fromTheta_,
			const float toTheta_,
			cref<color> color_
		);

	public:
		void drawText(
			math::vec2 pos_,
			cref<string_view> text_,
			_In_ ref<Font> font_,
			const float fontSize_,
			cref<color> color_
		);

		void drawImage(
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
		);

		void drawImageAsync(
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
		);
	};
}
