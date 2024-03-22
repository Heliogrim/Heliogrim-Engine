#include "EditorTextureFeedback.hpp"

using namespace ::hg::editor::gfx::loader;
using namespace ::hg;

EditorTextureFeedback::EditorTextureFeedback() = default;

EditorTextureFeedback::~EditorTextureFeedback() = default;

EditorTextureFeedback::response_type::type EditorTextureFeedback::operator()(
	mref<loader_traits::request::type> request_,
	mref<loader_traits::request::options> options_,
	cref<next_type> next_
) const {
	return next_(std::move(request_), std::move(options_));
}

EditorTextureFeedback::stream_response_type::type EditorTextureFeedback::operator()(
	mref<loader_traits::stream_request::type> request_,
	mref<loader_traits::stream_request::options> options_,
	cref<next_type> next_
) const {
	return next_(std::move(request_), std::move(options_));
}
