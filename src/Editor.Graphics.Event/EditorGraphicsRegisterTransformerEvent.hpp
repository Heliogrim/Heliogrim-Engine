#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Event/Event.hpp>

namespace hg::editor::graphics::loader {
	class EditorTextureTransformer;
}

namespace hg {
	class EditorGraphicsRegisterTransformerEvent final :
		public StatefulEvent {
	public:
		using this_type = EditorGraphicsRegisterTransformerEvent;

	public:
		inline static constexpr event_type_id typeId { "EditorGraphicsRegisterTransformerEvent"_typeId };

	public:
		explicit EditorGraphicsRegisterTransformerEvent(
			_In_ ref<editor::graphics::loader::EditorTextureTransformer> proxy_
		) noexcept;

		constexpr EditorGraphicsRegisterTransformerEvent(cref<this_type> other_) noexcept = default;

		constexpr EditorGraphicsRegisterTransformerEvent(mref<this_type> other_) noexcept = default;

		constexpr ~EditorGraphicsRegisterTransformerEvent() noexcept override = default;

	public:
		[[nodiscard]] bool canceled() const noexcept override;

		void cancel() noexcept override;

	private:
		nmpt<editor::graphics::loader::EditorTextureTransformer> _proxy;

	public:
		[[nodiscard]] ref<editor::graphics::loader::EditorTextureTransformer> getProxy() const noexcept;
	};
}
