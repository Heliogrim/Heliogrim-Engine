#include "Resource.hpp"

import Engine.Common;
import Engine.Graphics;
import Engine.Reflect;

using namespace hg::engine::graphics;
using namespace hg::engine::render;
using namespace hg;

bool hg::engine::render::Resource::isUniformBufferView() const noexcept {
    return view && IsType<UniformBufferView>(*view);
}

nmpt<const UniformBufferView> hg::engine::render::Resource::asUniformBufferView() const noexcept {
    return Cast<UniformBufferView>(view.get());
}

bool hg::engine::render::Resource::isStorageBufferView() const noexcept {
    return view && IsType<StorageBufferView>(*view);
}

nmpt<const StorageBufferView> hg::engine::render::Resource::asStorageBufferView() const noexcept {
    return Cast<StorageBufferView>(view.get());
}

bool hg::engine::render::Resource::isTexture() const noexcept {
    return view && IsType<TextureLikeObject>(*view);
}

nmpt<const TextureLikeObject> hg::engine::render::Resource::asTexture() const noexcept {
    return Cast<TextureLikeObject>(view.get());
}

bool hg::engine::render::Resource::isSampler() const noexcept {
    return view && IsType<TextureSampler>(*view);
}

nmpt<const TextureSampler> hg::engine::render::Resource::asSampler() const noexcept {
    return Cast<TextureSampler>(view.get());
}
