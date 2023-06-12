#pragma once

#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace hg::engine::gfx::material {
    class MaterialPrototypeParameter {
    public:
        using this_type = MaterialPrototypeParameter;

    public:
        MaterialPrototypeParameter(mref<string> uniqueName_) noexcept;

        ~MaterialPrototypeParameter() noexcept;

    private:
        string _uniqueName;

    public:
        [[nodiscard]] string getUniqueName() const noexcept;
    };
}
