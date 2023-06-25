#include "ModuleBuilder.hpp"

using namespace hg::engine::gfx::acc;
using namespace hg;

non_owning_rptr<const Tokenizer> ModuleBuilder::getTokenizer() const noexcept {
    return _tokenizer;
}

void ModuleBuilder::setTokenizer(mref<non_owning_rptr<const Tokenizer>> tokenizer_) {
    _tokenizer = _STD move(tokenizer_);
}
