#pragma once

#include "../NativeWindow.hpp"

struct SDL_Window;

namespace hg::engine::platform {
    class Win32Window final :
        public NativeWindow {
    public:
        Win32Window(
            const ptr<SDL_Window> sdlWnd_,
            const NativeWindowFlags flags_,
            const math::ivec2 offset_,
            const math::ivec2 size_,
            const string_view title_
        );

        ~Win32Window() noexcept override;

    private:
        ptr<SDL_Window> _sdlWnd;

    public:
        [[nodiscard]] const non_owning_rptr<SDL_Window> sdl() const noexcept;

    private:
        NativeWindowFlags _flags;

    public:
        [[nodiscard]] NativeWindowFlags flags() const noexcept override;

    private:
        math::ivec2 _offset;
        math::ivec2 _size;

    public:
        [[nodiscard]] math::ivec2 offset() const noexcept override;

        [[nodiscard]] math::ivec2 size() const noexcept override;

        void setOffset(const math::ivec2 offset_);

        void setSize(const math::ivec2 size_);

    private:
        string _title;

    public:
        [[nodiscard]] string_view title() const noexcept override;

        void setTitle(const string_view title_) override;
    };
}
