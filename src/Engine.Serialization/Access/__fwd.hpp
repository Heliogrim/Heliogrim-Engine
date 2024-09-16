#pragma once

#define ACCESS_LAYOUT template <typename> friend class ::hg::engine::serialization::access::Layout;
#define ACCESS_STRUCTURE template <class, class> friend class ::hg::engine::serialization::access::Structure;

namespace hg::engine::serialization::access {
	template <class Type_>
	class Layout;

	template <class Type_, class Scheme_>
	class Structure;
}
