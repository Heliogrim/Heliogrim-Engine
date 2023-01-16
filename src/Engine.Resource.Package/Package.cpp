#include "Package.hpp"

#include "Linker/PackageLinker.hpp"

using namespace ember::engine::resource;
using namespace ember;

const non_owning_rptr<PackageLinker> Package::getLinker() const noexcept {
    return _linker.get();
}
