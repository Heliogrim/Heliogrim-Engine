#include "PassCompiler.hpp"

using namespace hg::engine::accel;
using namespace hg;

non_owning_rptr<const Tokenizer> PassCompiler::getTokenizer() const noexcept {
    return _tokenizer;
}

void PassCompiler::setTokenizer(mref<non_owning_rptr<const Tokenizer>> tokenizer_) {
    _tokenizer = std::move(tokenizer_);
}
