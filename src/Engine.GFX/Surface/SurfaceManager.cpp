#include "SurfaceManager.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Platform/NativeWindow.hpp>
#include <Engine.Logging/Logger.hpp>

#ifdef _WIN32
#include <Engine.Platform/Windows/Win32Window.hpp>
#else
#include <Engine.Platform/Linux>
#endif

#include "Surface.hpp"

using namespace hg::engine::gfx;
using namespace hg::engine::platform;
using namespace hg;

SurfaceManager::SurfaceManager(const non_owning_rptr<Application> application_) :
    _application(application_),
    _surfaces() {}

SurfaceManager::~SurfaceManager() {
    tidy();
}

void SurfaceManager::tidy() {

    for (auto&& pair : _surfaces) {
        destroySurface(_STD move(pair.second));
    }

    _surfaces.clear();
}

non_owning_rptr<Surface> SurfaceManager::getSurface(const non_owning_rptr<platform::NativeWindow> window_) {
    const auto iter { _surfaces.find(window_) };
    if (iter != _surfaces.end()) {
        return iter->second.get();
    }
    return nullptr;
}

non_owning_rptr<Surface> SurfaceManager::makeSurface(mref<uptr<platform::NativeWindow>> window_) {

    if (_surfaces.contains(window_.get())) {
        return nullptr;
    }

    auto surface { make_uptr<Surface>(_STD move(window_), _application) };
    const auto result = _surfaces.insert_or_assign(surface->getNativeWindow(), _STD move(surface));

    if (not result.second) {
        // TODO: This is invalid behaviour; Need to check what insert_or_assign does on duplicated entry
        IM_CORE_ERROR("Tried to create multiple a surfaces for a single native window");
        destroySurface(_STD move(surface));
        return nullptr;
    }

    /**/

    /*
    result.first->first->resizeEmitter().on([this](cref<platform::PlatformResizeEvent> event_) {
        const auto* const surface { _surfaces.at(event_.getWindow()).get() };
    });
     */

    /**/

    return result.first->second.get();
}

bool SurfaceManager::destroySurface(mref<uptr<Surface>> surface_) {

    if (not surface_) {
        return false;
    }

    surface_->destroy();
    surface_.reset();

    return false;
}

bool SurfaceManager::destroySurface(const non_owning_rptr<platform::NativeWindow> window_) {

    const auto iter { _surfaces.find(window_) };
    if (iter == _surfaces.end()) {
        return false;
    }

    const auto result { destroySurface(_STD move(iter->second)) };
    if (result) {
        _surfaces.erase(iter);
    }

    return result;
}

bool SurfaceManager::destroySurface(mref<non_owning_rptr<Surface>> surface_) {

    const auto iter {
        _STD ranges::find_if(
            _surfaces,
            [surface_](const auto& entry_) {
                return entry_.second.get() == surface_;
            }
        )
    };

    if (iter == _surfaces.end()) {
        return false;
    }

    const auto result { destroySurface(_STD move(iter->second)) };
    if (result) {
        _surfaces.erase(iter);
    }

    return result;
}
