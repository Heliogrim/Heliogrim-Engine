#include "LoadPipeline.hpp"

#include <Engine.Common/StringLiterals.hpp>
#include <Engine.Common/__macro.hpp>

import Engine.Common;

using namespace hg;

static auto stage0 = [](StringView data_) {
	return String { data_ };
};

static auto stage1 = [add = 20.](mref<String> data_) {
	return std::stod(data_.c_str()) + add;
};

static auto stage2 = [](double data_) {
	return std::to_string(data_);
};

void test() {
	auto pipe = engine::resource::LoadPipeline(
		clone(stage2),
		clone(stage1),
		clone(stage0)
	);

	auto result = pipe("Test"sv);
}
