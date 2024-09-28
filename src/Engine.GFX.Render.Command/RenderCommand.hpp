#pragma once

#include <Engine.Common/Sal.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "RenderCommandTranslator.hpp"

namespace hg::engine::render::cmd {
	class macro_novtable RenderCommand {
	public:
		using this_type = RenderCommand;

	public:
		constexpr RenderCommand() noexcept = default;

		virtual constexpr ~RenderCommand() noexcept = default;

	public:
		nmpt<RenderCommand> next;

	public:
		virtual void operator()(
			_In_ ptr<RenderCommandTranslator::State> state_,
			_In_ ptr<RenderCommandTranslator> translator_
		) const noexcept = 0;
	};

	template <typename Type_>
	concept IsRenderCommand = std::derived_from<Type_, RenderCommand> &&
		std::is_nothrow_destructible_v<Type_> &&
		requires(const Type_& obj_) {
			obj_.operator()(
				std::declval<ptr<RenderCommandTranslator::State>>(),
				std::declval<ptr<RenderCommandTranslator>>()
			);
		};
}
