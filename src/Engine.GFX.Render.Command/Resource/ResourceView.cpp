#include "ResourceView.hpp"

#include <Engine.Common/Memory/MemoryPointer.hpp>
#include <Engine.GFX/Buffer/UniformBufferView.hpp>
#include <Engine.GFX/Buffer/StorageBufferView.hpp>
#include <Engine.GFX/Texture/SampledTextureView.hpp>
#include <Engine.Reflect/Cast.hpp>
#include <Engine.Reflect/IsType.hpp>

using namespace hg::engine::gfx;
using namespace hg::engine::render;
using namespace hg;

bool ResourceView::isUniformBufferView() const noexcept {
    return view && IsType<UniformBufferView>(*view);
}

nmpt<const UniformBufferView> ResourceView::asUniformBufferView() const noexcept {
    return Cast<UniformBufferView>(view.get());
}

bool ResourceView::isStorageBufferView() const noexcept {
    return view && IsType<StorageBufferView>(*view);
}

nmpt<const StorageBufferView> ResourceView::asStorageBufferView() const noexcept {
    return Cast<StorageBufferView>(view.get());
}

bool ResourceView::isTextureView() const noexcept {
    return view && IsType<SampledTextureView>(*view);
}

nmpt<const SampledTextureView> ResourceView::asTextureView() const noexcept {
    return Cast<SampledTextureView>(view.get());
}
