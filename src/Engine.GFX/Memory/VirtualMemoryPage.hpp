#pragma once
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

#include "VirtualMemoryPageState.hpp"

namespace ember::engine::gfx {
    struct AllocatedMemory;
    class VirtualMemory;
}

namespace ember::engine::gfx {

    class VirtualMemoryPage {
    public:
        using this_type = VirtualMemoryPage;

    public:
        VirtualMemoryPage(const non_owning_rptr<VirtualMemory> owner_, const u64 offset_, const u64 size_);

        ~VirtualMemoryPage();

    private:
        const non_owning_rptr<VirtualMemory> _owner;

    public:
        [[nodiscard]] const non_owning_rptr<VirtualMemory> owner() const noexcept;

    private:
        u64 _offset;

    public:
        [[nodiscard]] u64 offset() const noexcept;

    private:
        u64 _size;

    public:
        [[nodiscard]] u64 size() const noexcept;

    private:
        volatile VirtualMemoryPageState _state;

    public:
        [[nodiscard]] VirtualMemoryPageState state() const noexcept;

    private:
        ptr<AllocatedMemory> _memory;

    public:
        [[nodiscard]] const ptr<AllocatedMemory> allocated() const noexcept;
    };

}
