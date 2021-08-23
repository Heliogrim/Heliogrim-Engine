#include "VirtualTexture.hpp"

using namespace ember::engine::gfx;
using namespace ember;

VirtualTexturePage::VirtualTexturePage() {}

VirtualTexturePage::~VirtualTexturePage() noexcept { }

bool VirtualTexturePage::resident() const noexcept {
    return false;
}

void VirtualTexturePage::allocate(cref<sptr<Device>> device_) {}

void VirtualTexturePage::release(cref<sptr<Device>> device_) { }

VirtualTexture::VirtualTexture() { }

VirtualTexture::~VirtualTexture() noexcept {}
