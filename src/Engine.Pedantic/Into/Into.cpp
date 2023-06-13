#include "Into.hpp"

#include <Engine.Common/Wrapper.hpp>
#include <Engine.Common/Concurrent/SharedMemoryReference.hpp>

using namespace hg;

template <typename Ty_>
struct src_t {
    template <typename Ex_>
    src_t<Ex_> into();
};

template <typename Ty_>
struct dst_t {
    template <typename Ex_>
    src_t<Ex_> into();
};

template <typename Ty_>
struct uw_t {
    template <typename Ex_>
    Ex_ into();
};

struct ex_t {
    ex_t into();
};

struct exc_t {
    ex_t into();
};

struct exd_t : public ex_t { };

struct wrap_t {
    template <typename Ex_>
    src_t<Ex_> into();
};

template <typename Ty_>
struct box_t {
    template <typename Ex_>
    box_t<Ex_> into();
};

void test() {

    {
        ex_t obj {};
        auto dst1 = into<ex_t>(_STD move(obj));
    }
    {
        exc_t obj {};
        auto dst = into<ex_t>(_STD move(obj));
    }
    {
        uw_t<int> obj {};
        auto dst = into<long>(_STD move(obj));
    }
    {
        src_t<int> obj {};
        auto dst1 = into<long>(_STD move(obj));
    }
    {
        wrap_t obj {};
        auto dst1 = into<long>(_STD move(obj));
    }
    {
        src_t<int> obj {};
        auto dst1 = into<src_t<int>>(_STD move(obj));
    }
    {
        src_t<int> obj {};
        auto dst1 = into<src_t<long>>(_STD move(obj));
    }
    {
        dst_t<int> obj {};
        auto dst = into<src_t<long>>(_STD move(obj));
    }
    {
        dst_t<int> obj {};
        auto dst = into<long>(_STD move(obj));
    }
    {
        int obj {};
        auto dst = into<long>(_STD move(obj));
    }
    {
        bool obj {};
        auto dst = into<long>(_STD move(obj));
    }
    {
        long obj {};
        auto dst = into<long>(_STD move(obj));
    }
    {
        long obj {};
        auto dst = into<bool>(_STD move(obj));
    }
    {
        long obj {};
        auto dst = into<float>(_STD move(obj));
    }
    {
        static_assert(IsSmrBiPointerCompatible<exd_t, ex_t>);
        static_assert(IsSmrBiPointerCompatible<ex_t, exd_t>);

        smr<ex_t> obj {};
        auto dst = into<exd_t>(_STD move(obj));

        static_assert(IsSmrBiPointerCompatible<ex_t, exd_t> && not IsSmrPointerCompatible<ex_t, exd_t>);

        auto d = obj.into<exd_t>();
    }
    {
        static_assert(IsSmrBiPointerCompatible<exd_t, ex_t>);
        static_assert(IsSmrBiPointerCompatible<ex_t, exd_t>);

        smr<exd_t> obj {};
        auto dst = into<ex_t>(_STD move(obj));

        auto d = obj.into<ex_t>();
    }
    {
        static_assert(is_into_supported<smr<ex_t>, smr<exd_t>>);
        auto sa1 = into<smr<ex_t>, smr<exd_t>>(smr<exd_t> {});

        static_assert(is_into_supported<smr<ex_t>, exd_t>);
        auto sa2 = into<smr<ex_t>, exd_t>(smr<exd_t> {});

        static_assert(is_into_supported<ex_t, smr<exd_t>>);
        auto sa3 = into<ex_t, smr<exd_t>>(smr<exd_t> {});

        static_assert(is_into_supported<ex_t, exd_t, smr>);
        auto sa4 = into<ex_t, exd_t, smr>(smr<exd_t> {});
    }
}
