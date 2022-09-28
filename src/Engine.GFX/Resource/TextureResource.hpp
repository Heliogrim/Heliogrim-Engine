#pragma once

#include <Engine.Resource/Manage/Resource.hpp>
#include <Engine.Resource/Manage/Streamable.hpp>

#include "../Texture/VirtualTextureView.hpp"

namespace ember::engine::gfx {

    class TextureResource final :
        public res::partial::Streamable<
            res::Resource
        > {
    public:
        using this_type = TextureResource;
        using underlying_type = res::Resource;

    public:
        TextureResource() = default;

        ~TextureResource() override = default;

    public:
        struct {
            uptr<VirtualTextureView> view;
            bool __pseudo_stored;
        } _payload;

    public:
        [[nodiscard]] loaded_flag_type loaded() const noexcept override {
            // TODO:

            if (_payload.view.get() == nullptr) {
                return false;
            }

            return _payload.__pseudo_stored;
        }

        [[nodiscard]] res::ManageGuard<Resource> acquire(const res::ResourceUsageFlags flags_) override {
            return {};
        }

        [[nodiscard]] bool try_acquire(ref<res::ManageGuard<Resource>> guard_,
            const res::ResourceUsageFlags flags_) noexcept override {
            return false;
        }

        void release(const res::ResourceUsageFlags flags_) override {}

    public:
        void streamLoad(const ptr<EmberObject> options_) override;

        void streamUnload(const ptr<EmberObject> options_) override;
    };
}
