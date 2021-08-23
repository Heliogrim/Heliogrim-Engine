#pragma once

#include "__macro.hpp"
#include "Wrapper.hpp"

namespace ember {

    template <class Ty, class... Args>
    ptr<Ty> make_ptr(Args&&... args_) {
        return new Ty(_STD forward<Args>(args_)...);
    }

    template <class Ty, class... Args>
    sptr<Ty> make_sptr(Args&&... args_) {
        return _STD make_shared<Ty>(_STD forward<Args>(args_)...);
    }

    template <class Ty, class... Args>
    uptr<Ty> make_uptr(Args&&... args_) {
        return _STD make_unique<Ty>(_STD forward<Args>(args_)...);
    }
}
