#pragma once

#include <memory>

namespace ember {
    template <class Ty>
    using ref = Ty&;

    template <class Ty>
    using cref = const Ty&;

    template <class Ty>
    using mref = Ty&&;

    template <class Ty>
    using ptr = Ty*;

    template <class Ty>
    using pointer = Ty*;

    template <class Ty>
    using sptr = _STD shared_ptr<Ty>;

    template <class Ty>
    using wptr = _STD weak_ptr<Ty>;

    template <class Ty>
    using uptr = _STD unique_ptr<Ty>;

    template <class Ty>
    using managed = _STD shared_ptr<Ty>;
}
