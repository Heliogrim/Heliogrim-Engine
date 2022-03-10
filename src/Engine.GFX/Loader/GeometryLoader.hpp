#pragma once

#include <Engine.Common/Url.hpp>
#include <Engine.Common/Concurrent/Future.hpp>

#include "../Buffer/Buffer.hpp"
#include "../Device/Device.hpp"
#include "../Geometry/Geometry.hpp"

namespace ember::engine::gfx::loader {

    class GeometryLoader {
    public:
        using this_type = _STD unique_ptr<GeometryLoader>;
        using const_reference_type = const GeometryLoader&;

        using future_type = ::ember::concurrent::future<Geometry>;

    protected:
        /**
         * Constructor
         *
         * @author Julius
         * @date 23.08.2020
         *
         * @param  device_ The device.
         */
        GeometryLoader(cref<sptr<Device>> device_);

    public:
        /**
         * Destructor
         *
         * @author Julius
         * @date 23.08.2020
         */
        virtual ~GeometryLoader() = default;

    protected:
        sptr<Device> _device;

    public:
        /**
         * Loads the data structure of the given Url
         *
         * @author Julius
         * @date 24.08.2020
         *
         * @param  url_ The Url mapping the data to load.
         *
         * @returns A future_type.
         */
        future_type load(const Url& url_) const;

    protected:
        [[nodiscard]] Buffer acquireBuffer(const u64 size_);
    };
}
