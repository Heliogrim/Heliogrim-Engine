#pragma once
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Reflect/Inherit/InheritBase.hpp>

#include "TextureLikeObject.hpp"
#include "TextureSampler.hpp"

namespace hg::engine::gfx {
    class SampledTextureView :
        public InheritBase<SampledTextureView> {
    public:
        using this_type = SampledTextureView;

    public:
        SampledTextureView(mref<nmpt<const TextureLikeObject>> tlo_, mref<nmpt<const TextureSampler>> so_) noexcept;

        template <typename Type_> requires _STD is_constructible_v<nmpt<const TextureLikeObject>, Type_>
        SampledTextureView(
            Type_&& obj_,
            mref<nmpt<const TextureSampler>> so_
        ) noexcept :
            SampledTextureView(nmpt<const TextureLikeObject>(_STD forward<Type_>(obj_)), _STD move(so_)) {}

        SampledTextureView(cref<this_type> other_) noexcept = default;

        SampledTextureView(mref<this_type> other_) noexcept = default;

        ~SampledTextureView() noexcept;

    private:
        nmpt<const TextureLikeObject> _tlo;
        nmpt<const TextureSampler> _so;

    public:
        [[nodiscard]] nmpt<const TextureLikeObject> object() const noexcept;

        [[nodiscard]] nmpt<const TextureSampler> samplerObject() const noexcept;
    };
}
