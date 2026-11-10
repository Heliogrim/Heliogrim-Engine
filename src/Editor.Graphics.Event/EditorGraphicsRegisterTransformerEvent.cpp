#include "EditorGraphicsRegisterTransformerEvent.hpp"

using namespace ::hg;

EditorGraphicsRegisterTransformerEvent::EditorGraphicsRegisterTransformerEvent(
	ref<editor::gfx::loader::EditorTextureTransformer> proxy_
) noexcept :
	StatefulEvent(),
	_proxy(std::addressof(proxy_)) {}

bool EditorGraphicsRegisterTransformerEvent::canceled() const noexcept {
	return false;
}

void EditorGraphicsRegisterTransformerEvent::cancel() noexcept {
	// __noop;
}

ref<editor::gfx::loader::EditorTextureTransformer> EditorGraphicsRegisterTransformerEvent::getProxy() const noexcept {
	return *_proxy.get();
}
