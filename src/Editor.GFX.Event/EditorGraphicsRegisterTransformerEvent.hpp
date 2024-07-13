#pragma once
#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Event/Event.hpp>

namespace hg::editor::gfx::loader {
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
			_In_ ref<editor::gfx::loader::EditorTextureTransformer> proxy_
		) noexcept;

		constexpr EditorGraphicsRegisterTransformerEvent(cref<this_type> other_) noexcept = default;

		constexpr EditorGraphicsRegisterTransformerEvent(mref<this_type> other_) noexcept = default;

		constexpr ~EditorGraphicsRegisterTransformerEvent() noexcept override = default;

	public:
		[[nodiscard]] bool canceled() const noexcept override;

		void cancel() noexcept override;

	private:
		nmpt<editor::gfx::loader::EditorTextureTransformer> _proxy;

	public:
		[[nodiscard]] ref<editor::gfx::loader::EditorTextureTransformer> getProxy() const noexcept;
	};
}
