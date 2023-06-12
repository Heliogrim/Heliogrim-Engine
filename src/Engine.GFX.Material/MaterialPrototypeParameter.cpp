#include "MaterialPrototypeParameter.hpp"

using namespace hg::engine::gfx::material;
using namespace hg;

MaterialPrototypeParameter::MaterialPrototypeParameter(mref<string> uniqueName_) noexcept :
    _uniqueName(_STD move(uniqueName_)) {}

MaterialPrototypeParameter::~MaterialPrototypeParameter() noexcept = default;

string MaterialPrototypeParameter::getUniqueName() const noexcept {
    return _uniqueName;
}
