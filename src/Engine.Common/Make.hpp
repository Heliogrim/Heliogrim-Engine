#pragma once

#include "__macro.hpp"
#include "Wrapper.hpp"

#include "Concurrent/SharedMemoryReference.hpp"
#include "Meta/Concept.hpp"

namespace hg {
    template <class Ty, class... Args>
    ptr<Ty> make_ptr(Args&&... args_) {
        return new Ty(_STD forward<Args>(args_)...);
    }

    /* Unique Pointer (STD) */

    template <class Ty_, class... Args_> requires _STD is_constructible_v<Ty_, Args_...>
    [[nodiscard]] inline uptr<Ty_> make_uptr(Args_&&... args_) {
        return _STD make_unique<Ty_>(_STD forward<Args_>(args_)...);
    }

    template <class Ty_, class... Args_> requires
        (not _STD is_constructible_v<Ty_, Args_...>) &&
        (_STD is_aggregate_v<Ty_>)
    [[nodiscard]] inline uptr<Ty_> make_uptr(Args_&&... args_) {
        return uptr<Ty_>(new Ty_ { _STD forward<Args_>(args_)... });
    }

    /*
    template <class Ty, class... Args>
    uptr<Ty> make_uptr(Args&&... args_) {
        return _STD make_unique<Ty>(_STD forward<Args>(args_)...);
    }
     */

    /* Shared Pointer (STD) */

    template <class Ty_, class... Args_> requires _STD is_constructible_v<Ty_, Args_...>
    sptr<Ty_> make_sptr(Args_&&... args_) {
        return _STD make_shared<Ty_>(_STD forward<Args_>(args_)...);
    }

    /*
    template <class Ty_, class... Args_> requires _STD is_constructible_v<sptr<Ty_>, Args_...>
    sptr<Ty_> make_sptr(Args_&&... args_) {
        return _STD make_shared<Ty_>(_STD forward<Args_>(args_)...);
    }
     */

    /* Shared Memory References */

    template <class Ty>
    smr<Ty> make_smr(mref<uptr<Ty>> unique_) {
        auto* const ctrl = new SharedMemoryReferenceCtrlBlock<Ty>(unique_.release());
        return ctrl->acq();
    }

    template <class Ty> requires decayed_as<Ty, Ty>
    smr<Ty> make_smr(mref<ptr<Ty>> ptr_) {
        auto* const ctrl = new SharedMemoryReferenceCtrlBlock<Ty>(_STD move(ptr_));
        return ctrl->acq();
    }

    template <class Ty_, class Tx_> requires
        (not _STD is_same_v<Ty_, Tx_>) &&
        (_STD is_nothrow_convertible_v<ptr<Tx_>, ptr<Ty_>>)
    smr<Ty_> make_smr(mref<ptr<Tx_>> ptrTx_) {
        const auto ctrl = new SharedMemoryReferenceCtrlBlock<Tx_>(_STD move(ptrTx_));
        return ctrl->acq().template into<Ty_>();
    }

    template <class Ty, class... Args> requires _STD is_constructible_v<Ty, Args...>
    smr<Ty> make_smr(Args&&... args_) {
        auto* const ctrl = new SharedMemoryReferenceCtrlBlock<Ty>(
            // TODO: Check whether we get problems with types which differ between aggregate and list construction e.g std::vector<...>
            new Ty { _STD forward<Args>(args_)... }
        );
        return ctrl->acq();
    }
}
