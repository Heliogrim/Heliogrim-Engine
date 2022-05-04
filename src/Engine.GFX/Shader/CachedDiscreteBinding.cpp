#include "CachedDiscreteBinding.hpp"

#include "DiscreteBindingPool.hpp"

using namespace ember::engine::gfx;
using namespace ember;

CachedDiscreteBinding::CachedDiscreteBinding(mref<shader::DiscreteBindingGroup> binding_) :
    _binding(_STD move(binding_)) {}

CachedDiscreteBinding::CachedDiscreteBinding(mref<CachedDiscreteBinding> other_) noexcept :
    _binding(_STD move(other_._binding)) {}

CachedDiscreteBinding::~CachedDiscreteBinding() {
    release();
}

ref<CachedDiscreteBinding> CachedDiscreteBinding::operator=(mref<CachedDiscreteBinding> other_) noexcept {

    if (_STD addressof(other_) != this) {
        release();
        _binding = _STD move(other_._binding);
    }

    return *this;
}

void CachedDiscreteBinding::release() {
    if (!_binding.vkSet()) {
        return;
    }

    _binding.pool()->free(_STD move(_binding));
}

ref<shader::DiscreteBindingGroup> CachedDiscreteBinding::binding() noexcept {
    return _binding;
}

ref<CachedDiscreteBinding> CachedDiscreteBinding::operator=(mref<shader::DiscreteBindingGroup> dbg_) {

    #ifdef _DEBUG
    if (_binding.vkSet() == dbg_.vkSet()) {
        DEBUG_SNMSG(false, "WARN", "Reassignment of DiscreteBindingGroup to cache object will cause use after release.")
    }
    #endif

    release();
    _binding = _STD move(dbg_);

    return *this;
}
