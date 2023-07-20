#pragma once

#include <algorithm>
#include <Engine.Common/__macro.hpp>
#include <Engine.Common/Collection/Vector.hpp>

namespace hg {
    namespace {
        template <class Iter_, class OutIter_, typename Pred_>
        constexpr void __ct_sort(Iter_ first_, Iter_ last_, OutIter_ out_, Pred_ pred_) {

            const auto distance = _STD distance(first_, last_);
            if (distance < 2) {
                return;
            }

            const auto partition = _STD next(first_, distance / 2);

            __ct_sort(first_, partition, out_, pred_);
            __ct_sort(partition, last_, _STD next(out_, distance / 2), pred_);

            _STD merge(first_, partition, partition, last_, out_, pred_);
            _STD copy(out_, _STD next(out_, distance), first_);
        }
    }

    template <typename Iter_, typename Pred_ = _STD less<_STD remove_cvref_t<decltype(*_STD declval<Iter_>())>>>
    constexpr void compile_sort(Iter_ first_, Iter_ last_, Pred_ pred_ = {}) {
        using Type = typename Iter_::value_type;
        Vector<Type> tmp(first_, last_);
        __ct_sort(first_, last_, tmp.begin(), pred_);
    }
}
