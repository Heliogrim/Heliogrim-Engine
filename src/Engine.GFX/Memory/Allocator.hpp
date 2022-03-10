#pragma once
#include <Engine.Common/Types.hpp>

#include "AllocatedMemory.hpp"

namespace ember::engine::gfx {

    class __declspec(novtable) Allocator {
    public:
        using this_type = Allocator;

    protected:
        Allocator() noexcept = default;

    public:
        virtual ~Allocator() noexcept = default;

    protected:
        u64 _align;

    public:
        [[nodiscard]] u64 align() const noexcept;

    public:
        [[nodiscard]] virtual ptr<AllocatedMemory> allocate(const u64 size_) = 0;

        virtual void free(mref<ptr<AllocatedMemory>> memory_) = 0;
    };
}
