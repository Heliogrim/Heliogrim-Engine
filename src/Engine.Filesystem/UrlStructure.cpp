#include "Url.hpp"

#include <Engine.Serialization/Access/Structure.hpp>
#include <Engine.Serialization/Structure/StringScopedSlot.hpp>
#include <Engine.Serialization/Structure/StructScopedSlot.hpp>

using namespace hg::engine::serialization;
using namespace hg::engine::fs;
using namespace hg;

template <>
void access::Structure<Url>::serialize(const Url& self_, mref<StructScopedSlot> slot_) {
	slot_.insertSlot<string>("scheme") << self_._scheme;
	slot_.insertSlot<string>("path") << static_cast<String>(self_._path);
}

template <>
void access::Structure<Url>::hydrate(cref<StructScopedSlot> slot_, Url& target_) {
	slot_.getSlot<string>("scheme") >> target_._scheme;

	string tmp {};
	slot_.getSlot<string>("path") >> tmp;
	target_.setPath(fs::Path { std::move(tmp) });
}
