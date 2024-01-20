#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/StableUnorderedMap.hpp>
#include <Engine.Common/Memory/MemoryPointer.hpp>

namespace hg::engine::platform {
    class NativeWindow;
}

namespace hg::engine::gfx {
    class Application;
    class Surface;
    class Swapchain;
}

namespace hg::engine::gfx {
    class SurfaceManager {
    public:
        using this_type = SurfaceManager;

    public:
        SurfaceManager(const non_owning_rptr<Application> application_);

        ~SurfaceManager();

    private:
        void tidy();

    private:
        const non_owning_rptr<Application> _application;

        StableUnorderedMap<
            non_owning_rptr<platform::NativeWindow>, uptr<Surface>
        > _surfaces;

    public:
        /**
         *
         * @returns A pointer to the bound surface, otherwise nullptr.
         */
        [[nodiscard]] nmpt<Surface> getSurface(const non_owning_rptr<platform::NativeWindow> window_);

        /**
         *
         * @returns A pointer to the newly create surface, otherwise nullptr.
         */
        [[nodiscard]] nmpt<Surface> makeSurface(mref<uptr<platform::NativeWindow>> window_);

    private:
        bool destroySurface(mref<uptr<Surface>> surface_);

    public:
        bool destroySurface(const non_owning_rptr<platform::NativeWindow> window_);

        bool destroySurface(mref<nmpt<Surface>> surface_);
    };
}
