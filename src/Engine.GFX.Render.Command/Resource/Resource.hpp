#pragma once
#include <type_traits>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Reflect/Inherit/ClassMetaBase.hpp>

namespace hg::engine::gfx {
    class UniformBufferView;
    class StorageBufferView;
    class TextureLikeObject;
    class TextureSampler;
}

namespace hg::engine::render {
    class Resource {
    public:
        using this_type = Resource;

    public:
        constexpr Resource() noexcept = default;

        template <typename Type_> requires _STD is_constructible_v<nmpt<const ClassMetaBase>, Type_>
        constexpr Resource(Type_&& view_) noexcept :
            view(_STD forward<Type_>(view_)) {}

        constexpr Resource(cref<this_type> other_) noexcept = default;

        constexpr Resource(mref<this_type> other_) noexcept = default;

        constexpr ~Resource() noexcept = default;

    public:
        ref<this_type> operator=(cref<this_type> other_) noexcept = default;

        ref<this_type> operator=(mref<this_type> other_) noexcept = default;

    public:
        nmpt<const ClassMetaBase> view;

    public:
        [[nodiscard]] constexpr bool equivalent(cref<this_type> other_) const noexcept {
            return view == other_.view;
        }

        [[nodiscard]] bool isUniformBufferView() const noexcept;

        nmpt<const gfx::UniformBufferView> asUniformBufferView() const noexcept;

        [[nodiscard]] bool isStorageBufferView() const noexcept;

        [[nodiscard]] nmpt<const gfx::StorageBufferView> asStorageBufferView() const noexcept;

        [[nodiscard]] bool isTexture() const noexcept;

        [[nodiscard]] nmpt<const gfx::TextureLikeObject> asTexture() const noexcept;

        [[nodiscard]] bool isSampler() const noexcept;

        [[nodiscard]] nmpt<const gfx::TextureSampler> asSampler() const noexcept;
    };
}
