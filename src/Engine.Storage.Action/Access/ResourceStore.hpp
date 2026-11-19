#pragma once

#include <Engine.Pedantic.Resource/AccessMode.hpp>

#if not defined(HG_MODULE_INTERFACE_UNIT)
import std;
#endif

namespace hg::engine::storage {
	template <typename MutableType_, typename Type_>
	class IoResource;

	template <class StoreType_, typename MutableType_, typename Type_ = std::add_const_t<MutableType_>>
	concept IsResourceStore = requires(const StoreType_& store_, const IoResource<MutableType_, Type_>& ior_) {
		{ store_.resolveMutable(ior_, ResourceAccessMode {}) } -> std::same_as<std::add_lvalue_reference_t<MutableType_>>;
		{ store_.resolveConstant(ior_, ResourceAccessMode {}) } -> std::same_as<std::add_lvalue_reference_t<Type_>>;
	};
}
