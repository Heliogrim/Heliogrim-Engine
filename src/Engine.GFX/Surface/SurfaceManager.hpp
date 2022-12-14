#pragma once

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Collection/StableUnorderedMap.hpp>

namespace ember::engine::platform {
    class NativeWindow;
}

namespace ember::engine::gfx {
    class Application;
    class Surface;
    class Swapchain;
}

namespace ember::engine::gfx {
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
        [[nodiscard]] non_owning_rptr<Surface> getSurface(const non_owning_rptr<platform::NativeWindow> window_);

        /**
         *
         * @returns A pointer to the newly create surface, otherwise nullptr.
         */
        [[nodiscard]] non_owning_rptr<Surface> makeSurface(mref<uptr<platform::NativeWindow>> window_);

    private:
        bool destroySurface(mref<uptr<Surface>> surface_);

    public:
        bool destroySurface(const non_owning_rptr<platform::NativeWindow> window_);

        bool destroySurface(mref<non_owning_rptr<Surface>> surface_);
    };
}
