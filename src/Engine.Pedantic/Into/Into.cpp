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

void test() {

    {
        src_t<int> obj {};
        auto dst1 = into<src_t<int>>(_STD move(obj));
    }
    {
        src_t<int> obj {};
        auto dst1 = into<long>(_STD move(obj));
    }
    {
        src_t<int> obj {};
        auto dst1 = into<src_t<long>>(_STD move(obj));
    }
    {
        dst_t<int> obj {};
        auto dst = into<long>(_STD move(obj));
    }
    {
        dst_t<int> obj {};
        auto dst = into<src_t<long>>(_STD move(obj));
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
}
