#pragma once
#include <type_traits>
#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.Reflect/Inherit/ClassMetaBase.hpp>

namespace hg::engine::gfx {
    class UniformBufferView;
    class StorageBufferView;
    class SampledTextureView;
}

namespace hg::engine::render {
    class ResourceView {
    public:
        using this_type = ResourceView;

    public:
        constexpr ResourceView() noexcept = default;

        template <typename Type_> requires _STD is_constructible_v<nmpt<const ClassMetaBase>, Type_>
        constexpr ResourceView(Type_&& view_) noexcept :
            view(_STD forward<Type_>(view_)) {}

        constexpr ResourceView(cref<this_type> other_) noexcept = default;

        constexpr ResourceView(mref<this_type> other_) noexcept = default;

        constexpr ~ResourceView() noexcept = default;

    public:
        ref<this_type> operator=(cref<this_type> other_) noexcept = default;

        ref<this_type> operator=(mref<this_type> other_) noexcept = default;

    public:
        nmpt<const ClassMetaBase> view;

    public:
        [[nodiscard]] bool isUniformBufferView() const noexcept;

        nmpt<const gfx::UniformBufferView> asUniformBufferView() const noexcept;

        [[nodiscard]] bool isStorageBufferView() const noexcept;

        [[nodiscard]] nmpt<const gfx::StorageBufferView> asStorageBufferView() const noexcept;

        [[nodiscard]] bool isTextureView() const noexcept;

        [[nodiscard]] nmpt<const gfx::SampledTextureView> asTextureView() const noexcept;
    };
}
