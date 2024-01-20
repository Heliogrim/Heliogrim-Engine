#include "Resource.hpp"

#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX/Buffer/UniformBufferView.hpp>
#include <Engine.GFX/Buffer/StorageBufferView.hpp>
#include <Engine.GFX/Texture/TextureLikeObject.hpp>
#include <Engine.GFX/Texture/TextureSampler.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Reflect/IsType.hpp>

using namespace hg::engine::gfx;
using namespace hg::engine::render;
using namespace hg;

bool Resource::isUniformBufferView() const noexcept {
    return view && IsType<UniformBufferView>(*view);
}

nmpt<const UniformBufferView> Resource::asUniformBufferView() const noexcept {
    return Cast<UniformBufferView>(view.get());
}

bool Resource::isStorageBufferView() const noexcept {
    return view && IsType<StorageBufferView>(*view);
}

nmpt<const StorageBufferView> Resource::asStorageBufferView() const noexcept {
    return Cast<StorageBufferView>(view.get());
}

bool Resource::isTexture() const noexcept {
    return view && IsType<TextureLikeObject>(*view);
}

nmpt<const TextureLikeObject> Resource::asTexture() const noexcept {
    return Cast<TextureLikeObject>(view.get());
}

bool Resource::isSampler() const noexcept {
    return view && IsType<TextureSampler>(*view);
}

nmpt<const TextureSampler> Resource::asSampler() const noexcept {
    return Cast<TextureSampler>(view.get());
}
