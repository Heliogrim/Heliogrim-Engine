#include "Guid.hpp"

#include <combaseapi.h>

using namespace hg;

#ifdef _WIN32
Guid::Guid(const _GUID& value_) :
    pre(value_.Data1),
    c0(value_.Data2),
    c1(value_.Data3),
    post(*reinterpret_cast<const u64* const>(value_.Data4)) {}
#endif

void hg::GuidGenerate(ref<Guid> guid_) {
    #ifdef _WIN32
    static_assert(sizeof(Guid) == sizeof(GUID));
    CoCreateGuid(reinterpret_cast<GUID*>(&guid_.bytes));
    #else
        #pragma error("Not implemented uuid generator.")
    #endif
}
