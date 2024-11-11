#include "MutationDynamicGen.hpp"

using namespace hg::engine::storage::system;
using namespace hg;

MutationDynamicGen::MutationDynamicGen(mref<InlineAutoArray<DynamicStage, 3>> stages_) noexcept :
	Mutation(),
	_stages(::hg::move(stages_)) {}
