#include "UniquePointer.hpp"

#include <Engine.Pedantic/Into/Into.hpp>

using namespace hg;

struct Ty0 {};

struct Ty1 : public Ty0 {};

static_assert(::hg::is_into_supported<std::unique_ptr<Ty0>, std::unique_ptr<Ty1>>);
static_assert(::hg::__is_into_impl_invocable<std::unique_ptr<Ty0>, std::unique_ptr<Ty1>>);
