#pragma once
#include <Engine.Resource/Manage/Resource.hpp>

#include "TextureResource.hpp"
#include "../Buffer/VirtualBufferView.hpp"

namespace ember::engine::gfx {

    class MaterialResource :
        public res::Resource {
    public:
        using this_type = MaterialResource;
        using underlying_type = res::Resource;

    public:
        MaterialResource() = default;

        ~MaterialResource() override = default;

    public:
        struct {
            ptr<TextureResource> diffuse;
            ptr<TextureResource> normal;
            ptr<TextureResource> roughness;
            ptr<TextureResource> metalness;
            ptr<TextureResource> ao;
            ptr<TextureResource> alpha;
            //
            uptr<VirtualBufferView> view;
            bool __pseudo_stored;
        } _payload;

    public:
        [[nodiscard]] loaded_flag_type loaded() const noexcept override;

        [[nodiscard]] res::ManageGuard<Resource> acquire(const res::ResourceUsageFlags flags_) override;

        [[nodiscard]] bool try_acquire(
            ref<res::ManageGuard<Resource>> guard_,
            const res::ResourceUsageFlags flags_
        ) noexcept override;

        void release(const res::ResourceUsageFlags flags_) override;
    };
}
