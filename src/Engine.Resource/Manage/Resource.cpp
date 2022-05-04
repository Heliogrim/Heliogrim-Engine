#include "Resource.hpp"

using namespace ember::engine::res;
using namespace ember;

bool Resource::isLoaded() const noexcept {
    return this->loaded() != 0;
}
