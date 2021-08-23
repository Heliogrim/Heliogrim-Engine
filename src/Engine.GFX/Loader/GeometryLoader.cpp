#include "GeometryLoader.hpp"

using namespace ember::engine::gfx::loader;
using namespace ember;

GeometryLoader::this_type GeometryLoader::_this = nullptr;

GeometryLoader::GeometryLoader(cref<sptr<Device>> device_) :
    _device(device_) {}

GeometryLoader::const_reference_type GeometryLoader::make(cref<sptr<Device>> device_) {
    if (!_this)
        _this = _STD move(
            _STD unique_ptr<GeometryLoader>(new GeometryLoader(device_))
        );
    return *(_this.get());
}

GeometryLoader::const_reference_type GeometryLoader::get() {
    return *(_this.get());
}

GeometryLoader::future_type GeometryLoader::load(const Url& url_) const {
    const future_type::state_type state = nullptr;
    return future_type { state };
}
