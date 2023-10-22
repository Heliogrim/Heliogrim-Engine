#pragma once
#include <bit>
#include <span>
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <tl/expected.hpp>

#include "RenderCommand.hpp"

namespace hg::engine::gfx::render::cmd {
    class AllocationError : _STD exception {};

    class RenderCommandAllocator {
    public:
        using this_type = RenderCommandAllocator;

        using size_type = size_t;
        using difference_type = ptrdiff_t;

    public:
        constexpr RenderCommandAllocator() noexcept = default;

        constexpr RenderCommandAllocator(mref<this_type> other_) noexcept = default;

        virtual ~RenderCommandAllocator() noexcept = default;

    protected:
        [[nodiscard]] virtual tl::expected<_STD span<_::byte>, AllocationError> allocateUnique(const size_type size_) =
        0;

        [[nodiscard]] virtual bool deallocateUnique(mref<_STD span<_::byte>> memory_) = 0;

    public:
        template <typename CommandType_, typename... Args_> requires
            IsRenderCommand<CommandType_> &&
            _STD is_constructible_v<CommandType_, Args_...>
        [[nodiscard]] tl::expected<const ptr<CommandType_>, AllocationError> allocateCommand(
            Args_&&... args_
        ) {
            const auto ar = allocateUnique(sizeof(CommandType_));
            if (ar.has_value()) {
                return new(ar.value().data())CommandType_(_STD forward<Args_>(args_)...);
            }
            return tl::expected<const ptr<CommandType_>, AllocationError> { tl::unexpect, ar.error() };
        }

        template <typename CommandType_> requires
            IsRenderCommand<_STD remove_cvref_t<CommandType_>> &&
            (_STD is_move_constructible_v<_STD remove_cvref_t<CommandType_>>)
        [[nodiscard]] tl::expected<const ptr<_STD remove_cvref_t<CommandType_>>, AllocationError> allocateCommand(
            CommandType_&& command_
        ) {
            const auto ar = allocateUnique(sizeof(CommandType_));
            if (ar.has_value()) {
                return new(ar.value().data())CommandType_(_STD forward<CommandType_>(command_));
            }
            return tl::expected<const ptr<_STD remove_cvref_t<CommandType_>>, AllocationError> {
                tl::unexpect, ar.error()
            };
        }
    };
}
