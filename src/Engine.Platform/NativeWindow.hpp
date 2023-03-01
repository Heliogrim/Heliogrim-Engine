#pragma once

#include <Engine.Common/Math/Extent.hpp>
#include <Engine.Common/Math/Vector.hpp>
#include <Engine.Common/String.hpp>
#include <Engine.Event/EventEmitter.hpp>

#include "NativeWindowFlags.hpp"
#include "Event/PlatformResizeEvent.hpp"

namespace hg::engine::platform {
    /**
     * TODO: Check whether we want to handle DPI and therefore new a certain granularity
     *  to determine where to apply the dpi and in which way.
     */
    class __declspec(novtable) NativeWindow {
    public:
        using this_type = NativeWindow;

    protected:
        NativeWindow();

    public:
        virtual ~NativeWindow() noexcept = default;

    public:
        [[nodiscard]] virtual NativeWindowFlags flags() const noexcept = 0;

    public:
        [[nodiscard]] virtual math::ivec2 offset() const noexcept = 0;

        [[nodiscard]] virtual math::ivec2 size() const noexcept = 0;

    public:
        [[nodiscard]] virtual string_view title() const noexcept = 0;

        virtual void setTitle(const string_view title_) = 0;

    private:
        StatelessEventEmitter<PlatformResizeEvent> _resizeEmitter;

    public:
        [[nodiscard]] cref<decltype(_resizeEmitter)> resizeEmitter() const noexcept;

        [[nodiscard]] ref<decltype(_resizeEmitter)> resizeEmitter() noexcept;
    };
}
