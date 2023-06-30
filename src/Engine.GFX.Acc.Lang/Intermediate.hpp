#pragma once
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

#include "IL.hpp"
#include "IR.hpp"

namespace hg::engine::gfx::acc::lang {
    class Intermediate {
    public:
        using this_type = Intermediate;

    public:
        Intermediate() noexcept = default;

        Intermediate(mref<uptr<IL>> il_, mref<uptr<IR>> ir_) noexcept;

        Intermediate(cref<this_type> other_);

        Intermediate(mref<this_type> other_) noexcept = default;

        ~Intermediate() noexcept = default;

    private:
        uptr<IL> _code;
        uptr<IR> _rep;

    public:
        [[nodiscard]] nmpt<const IL> getIl() const noexcept;

        void setIl(mref<uptr<IL>> il_);

        [[nodiscard]] nmpt<const IR> getIr() const noexcept;

        void setIr(mref<uptr<IR>> ir_);
    };
}
