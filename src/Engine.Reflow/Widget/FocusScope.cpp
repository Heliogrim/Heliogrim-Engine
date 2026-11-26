#include "FocusScope.hpp"

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Reflect;
#endif

using namespace hg::engine::reflow;
using namespace hg;

FocusScope::FocusScope(const FocusScopeKind kind_) :
	CompoundWidget(ReflowClassList {}, nullptr),
	_kind(kind_),
	_remembered(),
	_context { .scope = this } {}

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

void FocusScope::setContent(const sptr<Widget> content_) {
	replaceSingleChild(*this, _children, content_);
}

ptr<void> FocusScope::providedContext(const type_id contextType_) noexcept {
	if (contextType_ == refl::ctid<FocusScopeContext>()) {
		return std::addressof(_context);
	}
	return CompoundWidget::providedContext(contextType_);
}
