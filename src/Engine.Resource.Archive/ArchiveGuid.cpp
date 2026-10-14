#include "ArchiveGuid.hpp"

#include <cstring>
#include <Engine.Common/Guid.hpp>

using namespace hg::engine::resource;
using namespace hg;

ArchiveGuid ArchiveGuid::random() noexcept {
	auto tmp = Guid {};
	GuidGenerate(tmp);
	return from(tmp);
}

ArchiveGuid ArchiveGuid::from(cref<Guid> guid_) noexcept {
	auto tmp = Array<_::byte, 16> {};
	guid_.hostToNetwork(tmp);

	auto res = ArchiveGuid {};
	std::memcpy(res.data(), tmp.data(), res.size());
	return res;
}

ArchiveGuid::operator Guid() const noexcept {
	auto tmp = Array<_::byte, 16> {};
	std::memcpy(tmp.data(), this->data(), this->size());
	return Guid::networkToHost(tmp);
}
