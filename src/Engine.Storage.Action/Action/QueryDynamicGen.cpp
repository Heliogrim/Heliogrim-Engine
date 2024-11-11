#include "QueryDynamicGen.hpp"

#include <Engine.Reflect/IsType.hpp>

using namespace hg::engine::storage::system;
using namespace hg::engine::storage;
using namespace hg;

QueryDynamicGen::QueryDynamicGen(mref<InlineAutoArray<DynamicStage, 3>> stages_) noexcept :
	Query(),
	_stages(::hg::move(stages_)) {}
