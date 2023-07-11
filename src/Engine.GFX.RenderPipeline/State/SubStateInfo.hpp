#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX/Cache/LocalCacheCtrl.hpp>
#include <Engine.GFX/Memory/LocalPooledAllocator.hpp>
#include <Engine.Logging/Logger.hpp>
#include <Engine.Common/Math/Hash.hpp>

namespace hg::engine::gfx::render::pipeline {
    template <typename Type_>
    concept SubStateType = _STD is_constructible_v<Type_> &&
        _STD is_move_constructible_v<Type_> &&
        _STD is_destructible_v<Type_>;

    struct SubStateInfo {
        size_t key;
        type_id type;
        size_t size;
        ptrdiff_t offset;
    };
}

namespace std {
    template <>
    struct hash<::hg::engine::gfx::render::pipeline::SubStateInfo> :
        _STD hash<size_t> {
        [[nodiscard]] size_t operator()(const ::hg::engine::gfx::render::pipeline::SubStateInfo& val_) const noexcept {
            using namespace hg::engine::gfx::render::pipeline;
            using namespace hg;

            const auto& hasher = static_cast<const _STD hash<size_t>&>(*this);

            size_t seed {};
            hg::hash::hash_combine(seed, hasher(val_.type.data));
            hg::hash::hash_combine(seed, hasher(val_.key));

            return seed;
        }
    };
}
