#pragma once
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Collection/Vector.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

#include "../TextureFormat.hpp"
#include "../Device/Device.hpp"
#include "../Surface/Surface.hpp"
#include "../Texture/Texture.hpp"

namespace hg::engine::gfx {
    struct SwapchainImage {
        /**/
        smr<Texture> image;
        /**/
        vk::Semaphore readySignal;
        /**/
        Vector<vk::Semaphore> presentWaits;
    };

    class __declspec(novtable) Swapchain {
    public:
        using value_type = Swapchain;

    protected:
        Swapchain() noexcept = default;

    public:
        virtual ~Swapchain() = default;

    public:
        virtual void setup(cref<sptr<Device>> device_);

        virtual void destroy() = 0;

    protected:
        sptr<Device> _device;

    protected:
        TextureFormat _format;

    public:
        [[nodiscard]] cref<TextureFormat> format() const noexcept;

    protected:
        math::uivec2 _extent;

    public:
        [[nodiscard]] cref<math::uivec2> extent() const noexcept;

    protected:
        Vector<SwapchainImage> _images;

    public:
        [[nodiscard]] virtual s64 imageCount() const noexcept;

        [[nodiscard]] cref<smr<Texture>> at(u64 idx_) const;

    public:
        virtual bool acquireNext(
            _Out_ ref<s64> idx_,
            _Out_ ref<smr<Texture>> image_,
            _Out_ ref<vk::Semaphore> signal_
        ) = 0;

        virtual vk::Result presentNext(_In_ u64 idx_) = 0;

        virtual vk::Result presentNext(_In_ u64 idx_, _In_ cref<Vector<vk::Semaphore>> waits_) = 0;

    public:
        virtual bool consumeNext(
            _Out_ ref<smr<Texture>> image_,
            _Out_ ref<vk::Semaphore> signal_,
            _Out_ ref<Vector<vk::Semaphore>> waits_
        ) = 0;
    };
}
