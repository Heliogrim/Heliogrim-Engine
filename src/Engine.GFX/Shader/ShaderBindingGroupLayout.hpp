#pragma once
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Wrapper.hpp>

namespace ember::engine::gfx::shader {

    struct ShaderBindingGroupLayout {
        u32 images;
        u32 uniforms;
        u32 storages;

        [[nodiscard]] bool operator==(cref<ShaderBindingGroupLayout> other_) const noexcept {
            return images == other_.images && uniforms == other_.uniforms && storages == other_.storages;
        }
    };
}

namespace std {
    template <>
    struct hash<ember::engine::gfx::shader::ShaderBindingGroupLayout> {
        [[nodiscard]] size_t operator()(
            ember::cref<ember::engine::gfx::shader::ShaderBindingGroupLayout> obj_
        ) const noexcept {
            size_t base { obj_.images };
            base += obj_.uniforms;
            base += obj_.storages;

            using namespace ember;
            return base |
                (static_cast<u64>(obj_.images) << 16ui64) |
                (static_cast<u64>(obj_.uniforms) << 32ui64) |
                (static_cast<u64>(obj_.storages) << 48ui64);
        }
    };
}
