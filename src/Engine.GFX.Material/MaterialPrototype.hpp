#pragma once

#include <Engine.Common/Guid.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/InlineAutoArray.hpp>
#include <Engine.Common/Collection/Set.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>
#include <tl/optional.hpp>

#include "MaterialEffect.hpp"
#include "__fwd.hpp"

namespace hg::engine::gfx::material {
    class MaterialPrototype {
    public:
        using this_type = MaterialPrototype;

    public:
        MaterialPrototype() noexcept = default;

        MaterialPrototype(
            mref<Guid> guid_,
            mref<string> name_,
            mref<InlineAutoArray<MaterialEffect>> effects_,
            mref<Vector<MaterialPrototypeParameter>> parameters_
        ) noexcept;

        MaterialPrototype(mref<this_type> other_) noexcept = default;

        ~MaterialPrototype();

    private:
        Guid _guid;

    public:
        [[nodiscard]] Guid getGuid() const noexcept;

    private:
        string _name;

    public:
        [[nodiscard]] string getName() const noexcept;

    private:
        InlineAutoArray<MaterialEffect> _materialEffects;

    public:
        [[nodiscard]] cref<InlineAutoArray<MaterialEffect>> getAccelerationEffects() const noexcept;

    private:
        Vector<MaterialPrototypeParameter> _parameters;

    public:
        [[nodiscard]] cref<Vector<MaterialPrototypeParameter>> getParameters() const noexcept;

        bool addParameter(mref<MaterialPrototypeParameter> param_);

        [[nodiscard]] tl::optional<cref<MaterialPrototypeParameter>> getParameter(
            string_view uniqueName_
        ) const noexcept;

        bool removeParameter(string_view uniqueName_);

    public:
        [[nodiscard]] smr<Material> instantiate() const noexcept;
    };
}
