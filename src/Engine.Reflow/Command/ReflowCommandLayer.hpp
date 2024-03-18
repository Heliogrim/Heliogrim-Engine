#pragma once

#include "ReflowCommandBuffer.hpp"
#include "../ReflowPassState.hpp"

namespace hg::engine::reflow {
	class ReflowCommandLayer final {
	public:
		using this_type = ReflowCommandLayer;

	public:
		ReflowCommandLayer(cref<ReflowPassState> self_, ref<ReflowCommandBuffer> buffer_) :
			_self(std::addressof(self_)),
			_buffer(std::addressof(buffer_)),
			_skipLayers(0u) {
			enter();
		}

		ReflowCommandLayer(cref<ReflowPassState> self_, ref<ReflowCommandBuffer> buffer_, const u8 skipLayer_) :
			_self(std::addressof(self_)),
			_buffer(std::addressof(buffer_)),
			_skipLayers(skipLayer_) {
			enter();
		}

		ReflowCommandLayer(cref<this_type>) = delete;

		ReflowCommandLayer(mref<this_type>) = delete;

		~ReflowCommandLayer() {
			leave();
		}

	public:
		ref<this_type> operator=(cref<this_type>) = delete;

		ref<this_type> operator=(mref<this_type>) = delete;

	private:
		ptr<const ReflowPassState> _self;
		ptr<ReflowCommandBuffer> _buffer;
		u8 _skipLayers;

	protected:
		void enter() const noexcept {

			const auto scissor = math::f32ext2d {
				_self->layoutSize.x, _self->layoutSize.y, _self->layoutOffset.x, _self->layoutOffset.y
			};

			_buffer->pushLayer(_skipLayers + 1u);
			_buffer->pushIntersectScissor(scissor);
		}

		void leave() const noexcept {
			_buffer->popScissor();
			_buffer->popLayer(_skipLayers + 1u);
		}
	};
}
