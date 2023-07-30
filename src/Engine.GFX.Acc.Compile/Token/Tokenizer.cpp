#include "Tokenizer.hpp"

#include <sstream>
#include <Engine.Common/Collection/Vector.hpp>

using namespace hg::engine::gfx::acc;
using namespace hg;

constexpr static string_view no_prefix { ""sv };
constexpr static string_view delim { "/"sv };

static Token generateToken(string_view prefix_, mref<Vector<string>> scopes_, string_view name_) {

    _STD stringstream ss {};
    ss << prefix_;

    for (const auto& scope : scopes_) {
        ss << scope << delim;
    }

    ss << name_;
    return Token::from(ss.str());
}

Tokenizer::Tokenizer(
    mref<string> prefix_,
    mref<string> inScope_,
    mref<string> bindScope_,
    mref<string> outScope_
) noexcept :
    _prefix(_STD move(prefix_)),
    _inScope(_STD move(inScope_)),
    _bindScope(_STD move(bindScope_)),
    _outScope(_STD move(outScope_)) {}

Token Tokenizer::expand(mref<Token> token_, cref<string> data_) const {
    _STD stringstream ss {};
    ss << token_.value << delim << data_;
    return Token::from(ss.str());
}

Token Tokenizer::generate(cref<StageInput> asi_) const {

    const bool isFwd = asi_.transferType == TransferType::eForward;

    if (isFwd) {
        return generateToken(
            _prefix,
            { _inScope },
            asi_.token.value
        );
    }

    return generateToken(
        _prefix,
        { _bindScope },
        asi_.token.value
    );
}

Token Tokenizer::generate(cref<StageOutput> aso_) const {

    const bool isFwd = aso_.transferType == TransferType::eForward;

    if (isFwd) {
        return generateToken(
            _prefix,
            { _outScope },
            aso_.token.value
        );
    }

    return generateToken(
        _prefix,
        { _bindScope },
        aso_.token.value
    );
}

Token Tokenizer::transformAccStageIn(cref<Token> src_, bool forwarding) const {
    if (forwarding) {
        return generateToken(
            _prefix,
            { _inScope },
            src_.value
        );
    }

    return generateToken(
        _prefix,
        { _bindScope },
        src_.value
    );
}

Token Tokenizer::transformAccStageOut(cref<Token> src_, const bool forwarding) const {

    if (forwarding) {
        return generateToken(
            _prefix,
            { _outScope },
            src_.value
        );
    }

    return generateToken(
        _prefix,
        { _bindScope },
        src_.value
    );
}

Token Tokenizer::transformAccStageInToOut(cref<Token> src_) const {

    const auto prefixSize = _prefix.size();
    const auto inScopeSize = _inScope.size();
    const auto delimSize = delim.size();

    return generateToken(_prefix, { _outScope }, src_.value.substr(prefixSize + inScopeSize + delimSize));
}

bool Tokenizer::isStageIn(cref<Token> token_, bool forwarding) const {

    if (forwarding && not token_.value.starts_with(_prefix)) {
        return false;
    }

    if (forwarding && not(string_view {
        token_.value.data() + _prefix.size(), token_.value.size() - _prefix.size()
    }.starts_with(_inScope))) {
        return false;
    }

    if (not forwarding && not(string_view {
        token_.value.data() + _prefix.size(), token_.value.size() - _prefix.size()
    }.starts_with(_bindScope))) {
        return false;
    }

    return true;
}

bool Tokenizer::isStageOut(cref<Token> token_, bool forwarding) const {

    if (forwarding && not token_.value.starts_with(_prefix)) {
        return false;
    }

    if (forwarding && not(string_view {
        token_.value.data() + _prefix.size(), token_.value.size() - _prefix.size()
    }.starts_with(_outScope))) {
        return false;
    }

    if (not forwarding && not(string_view {
        token_.value.data() + _prefix.size(), token_.value.size() - _prefix.size()
    }.starts_with(_bindScope))) {
        return false;
    }

    return true;
}
