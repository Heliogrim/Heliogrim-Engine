#pragma once

#include <Engine.Resource/Manage/Resource.hpp>

#include "Engine.Reflow/Font/Font.hpp"

namespace ember::engine::gfx {

    class FontResource final :
        public res::Resource {
    public:
        using this_type = FontResource;
        using underlying_type = res::Resource;

    public:
        FontResource();

        ~FontResource() override;

    private:
        void tidy();

    public:
        ptr<reflow::Font> _fontData;

    public:
        [[nodiscard]] loaded_flag_type loaded() const noexcept override;

        [[nodiscard]] res::ManageGuard<Resource> acquire(const res::ResourceUsageFlags flags_) override;

        [[nodiscard]] bool try_acquire(ref<res::ManageGuard<Resource>> guard_,
            const res::ResourceUsageFlags flags_) noexcept override;

        void release(const res::ResourceUsageFlags flags_) override;
    };

}
