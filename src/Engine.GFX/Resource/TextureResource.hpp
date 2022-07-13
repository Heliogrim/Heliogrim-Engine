#pragma once

#include <Engine.Resource/Manage/Resource.hpp>

#include "../Texture/VirtualTextureView.hpp"

namespace ember::engine::gfx {

    class TextureResource final :
        public res::Resource {
    public:
        using this_type = TextureResource;
        using underlying_type = res::Resource;

    public:
        TextureResource() = default;

        ~TextureResource() override = default;

    public:
        struct {
            uptr<VirtualTextureView> view;
        } _payload;

    public:
        [[nodiscard]] loaded_flag_type loaded() const noexcept override {
            // TODO: 
            return _payload.view.get() != nullptr;
        }

        [[nodiscard]] res::ManageGuard<Resource> acquire(const res::ResourceUsageFlags flags_) override {
            return {};
        }

        [[nodiscard]] bool try_acquire(ref<res::ManageGuard<Resource>> guard_,
            const res::ResourceUsageFlags flags_) noexcept override {
            return false;
        }

        void release(const res::ResourceUsageFlags flags_) override {}
    };
}
