#pragma once
#include <Engine.Common/Types.hpp>
#include <Engine.Common/Collection/Vector.hpp>

#include "../TextureFormat.hpp"
#include "../Device/Device.hpp"
#include "../Surface/Surface.hpp"
#include "../Texture/Texture.hpp"

namespace ember::engine::gfx {

    /**
     * A swap image.
     *
     * @author Julius
     * @date 17.11.2020
     */
    struct SwapImage {
        vk::Image image;
        vk::ImageView view;

        vk::Framebuffer frame;
    };

    class Swapchain {
    public:
        using value_type = Swapchain;
        using reference_type = Swapchain&;
        using const_reference_type = const Swapchain&;

    protected:
        /**
         * Default constructor
         *
         * @author Julius
         * @date 08.12.2020
         */
        Swapchain() noexcept = default;

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 30.01.2021
         */
        virtual ~Swapchain() = default;

        /**
         * Setups this 
         *
         * @author Julius
         * @date 09.11.2020
         */
        virtual void setup() = 0;

        /**
         * Destroys this 
         *
         * @author Julius
         * @date 09.11.2020
         */
        virtual void destroy() = 0;

    protected:
        /**
         * Format
         */
        TextureFormat _format;

    public:
        /**
         * Gets the image format of the swapchain
         *
         * @author Julius
         * @date 19.11.2020
         *
         * @returns The formatted value.
         */
        [[nodiscard]] TextureFormat format() const noexcept;

    protected:
        /**
         * Extent { width, height }
         */
        math::uivec2 _extent;

    public:
        /**
         * Gets the extent
         *
         * @author Julius
         * @date 30.01.2021
         *
         * @returns A cref&lt;math::uivec2&gt;
         */
        [[nodiscard]] cref<math::uivec2> extent() const noexcept;

        /**
         * Gets the width
         *
         * @author Julius
         * @date 19.11.2020
         *
         * @returns A clockwork::__internal::types::uivec2::value_type.
         */
        [[nodiscard]] math::uivec2::value_type width() const noexcept;

        /**
         * Gets the height
         *
         * @author Julius
         * @date 19.11.2020
         *
         * @returns A clockwork::__internal::types::uivec2::value_type.
         */
        [[nodiscard]] math::uivec2::value_type height() const noexcept;

    protected:
        /**
         * Swap Images
         */
        Vector<sptr<Texture>> _images;

    public:

        /**
         * Gets the Texture / Swap Image at the given index
         *
         * @author Julius
         * @date 30.01.2021
         *
         * @param  idx_ Zero-based index of the.
         *
         * @returns A cref&lt;sptr&lt;Texture&gt;&gt;
         */
        [[nodiscard]] cref<sptr<Texture>> at(const u32 idx_) const noexcept;

        /**
         * Gets the length
         *
         * @author Julius
         * @date 30.01.2021
         *
         * @returns An u32.
         */
        [[nodiscard]] u32 length() const noexcept;

    private:
        /**
         * Current Idx
         */
        _STD atomic<u32> _idx;

    public:

        /**
         * Current index
         *
         * @author Julius
         * @date 31.01.2021
         *
         * @returns An u32.
         */
        [[nodiscard]] u32 currentIdx() const noexcept;

        /**
         * Sets current index
         *
         * @author Julius
         * @date 31.01.2021
         *
         * @param  idx_ Zero-based index of the.
         */
        void setCurrentIdx(u32 idx_) noexcept;
    };
}
