#pragma once

#include "__macro.hpp"
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

    template <typename Type_>
    using non_owning_rptr = Type_*;

    template <class Ty>
    using sptr = _STD shared_ptr<Ty>;

    template <class Ty>
    using wptr = _STD weak_ptr<Ty>;

    template <class Ty>
    using uptr = _STD unique_ptr<Ty>;

    template <class Ty>
    using managed = _STD shared_ptr<Ty>;

    /**/

    template <typename>
    class SharedMemoryReference;

    template <typename Ty>
    using smr = SharedMemoryReference<Ty>;

    /* C++ Language Extensions */

    /**
     * Helper wrapper which will implicitly apply the restricted attribute to the requested pointer type
     *
     * @note The `__restrict` specification is provided either from MSVC or the alias macro of `__macro.hpp`
     */
    template <typename Type_>
    using __restricted_ptr = Type_* __restrict;
}
