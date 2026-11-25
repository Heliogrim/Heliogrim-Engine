#include "FocusScope.hpp"

using namespace hg::engine::reflow;
using namespace hg;

FocusScope::FocusScope(const FocusScopeKind kind_) :
	CompoundWidget(ReflowClassList {}, nullptr),
	_kind(kind_),
	_remembered() {}

FocusScope::~FocusScope() = default;

string FocusScope::getTag() const noexcept {
	return std::format(R"(FocusScope <{:#x}>)", reinterpret_cast<u64>(this));
}

FocusScopeKind FocusScope::kind() const noexcept {
	return _kind;
}

sptr<Widget> FocusScope::remembered() const noexcept {
	auto widget = _remembered.lock();
	for (auto next = widget; next != nullptr; next = next->parent()) {
		if (next.get() == this) {
			return widget;
		}
	}
	return nullptr;
}

void FocusScope::setContent(sptr<Widget> content_) {
	content_->setParent(shared_from_this());
	_children.setChild(content_);
	markAsPending(true, true);
}
