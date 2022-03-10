#pragma once

#include <Engine.Common/Url.hpp>
#include <Engine.Common/Concurrent/Promise.hpp>
#include <gli/gli.hpp>

#include "../Device/Device.hpp"
#include "../Texture/Texture.hpp"

namespace ember::engine::gfx::loader {
    class TextureLoader {
    public:
        using value_type = TextureLoader;
        using reference_type = TextureLoader&;
        using const_reference_type = const TextureLoader&;

        using this_type = _STD unique_ptr<TextureLoader>;

        /**
         * Destructor
         *
         * @author Julius
         * @date 23.08.2020
         */
        ~TextureLoader() = default;

        /**
         * Loads the given URL
         *
         * @author Julius
         * @date 24.08.2020
         *
         * @param  url_ The URL to load.
         *
         * @returns A concurrent::promise&lt;Texture&gt;
         */
        ember::concurrent::promise<Texture> load(const Url& url_) const;

        Texture __tmp__load(const Url& url_);

    protected:
        sptr<Device> _device;

        /**
         * Constructor
         *
         * @author Julius
         * @date 23.08.2020
         *
         * @param  device_ The device.
         */
        TextureLoader(cref<sptr<Device>> device_);
    };
}
