#include "Diagnostic.hpp"

using namespace hg::engine::accel::lang;
using namespace hg;

ir::SourceSpan ir::pointAt(StringView file_, SourceLoc loc_) {
	return ir::SourceSpan { String(file_), loc_.offset, loc_.offset + loc_.length, loc_.line, loc_.column };
}
