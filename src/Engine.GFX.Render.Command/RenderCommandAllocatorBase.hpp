#pragma once
#include <bit>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <tl/expected.hpp>

#include "RenderCommand.hpp"

namespace hg::engine::gfx::render::cmd {
    class AllocationError : _STD exception {};

    class RenderCommandAllocatorBase {
    public:
        using this_type = RenderCommandAllocatorBase;

        using size_type = size_t;
        using difference_type = ptrdiff_t;

    protected:
        constexpr RenderCommandAllocatorBase() noexcept = default;

        constexpr RenderCommandAllocatorBase(mref<this_type> other_) noexcept = default;

    public:
        virtual ~RenderCommandAllocatorBase() noexcept = default;

    protected:
        [[nodiscard]] virtual void* allocateUnique(const size_type size_) = 0;

        [[nodiscard]] virtual bool deallocateUnique(mref<void*> memory_) = 0;

    public:
        template <typename CommandType_, typename... Args_> requires
            IsRenderCommand<CommandType_> &&
            _STD is_constructible_v<CommandType_, Args_...>
        [[nodiscard]] tl::expected<const ptr<CommandType_>, AllocationError> allocateCommand(
            Args_&&... args_
        ) {
            if (const auto ar = allocateUnique(sizeof(CommandType_))) {
                return new(ar)CommandType_(_STD forward<Args_>(args_)...);
            }
            return tl::make_unexpected(AllocationError {});
        }

        template <typename CommandType_> requires
            IsRenderCommand<_STD remove_cvref_t<CommandType_>> &&
            (_STD is_move_constructible_v<_STD remove_cvref_t<CommandType_>>)
        [[nodiscard]] tl::expected<const ptr<_STD remove_cvref_t<CommandType_>>, AllocationError> allocateCommand(
            CommandType_&& command_
        ) {
            const auto ar = allocateUnique(sizeof(CommandType_));
            if (ar) {
                return new(ar)CommandType_(_STD forward<CommandType_>(command_));
            }
            return tl::make_unexpected(AllocationError {});
        }
    };
}
