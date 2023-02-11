#pragma once

#include "__macro.hpp"
#include "Wrapper.hpp"

#include "Concurrent/SharedMemoryReference.hpp"
#include "Meta/Concept.hpp"

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

    /* Shared Memory References */

    template <class Ty, class... Args>
    smr<Ty> make_smr(Args&&... args_);

    /**/

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

    template <class Ty, class... Args>
    smr<Ty> make_smr(Args&&... args_) {
        auto* const ctrl = new SharedMemoryReferenceCtrlBlock<Ty>(new Ty(_STD forward<Args>(args_)...));
        return ctrl->acq();
    }
}
