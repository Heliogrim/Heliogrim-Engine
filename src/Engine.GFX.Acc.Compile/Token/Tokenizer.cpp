#include "Tokenizer.hpp"

#include <sstream>

using namespace hg::engine::gfx::acc;
using namespace hg;

constexpr static string_view no_prefix { ""sv };

static Token generateToken(string_view prefix_, mref<Vector<string>> scopes_, string_view name_) {

    constexpr string_view delim { "/"sv };

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

Token Tokenizer::generate(cref<InputLayoutDescription> ild_) const {
    return generateToken(
        _prefix,
        { _inScope },
        ild_.token.value
    );
}

Token Tokenizer::generate(cref<BindingLayoutDescription> bld_) const {

    if (bld_.bindingMode == DataBindingMode::eDynamic) {
        return generateToken(
            _prefix,
            { _bindScope },
            bld_.token.value
        );
    }

    return generateToken(
        no_prefix,
        { _bindScope },
        bld_.token.value
    );
}

Token Tokenizer::generate(cref<OutputLayoutDescription> old_) const {
    return generateToken(
        no_prefix,
        { _outScope },
        old_.token.value
    );
}

Token Tokenizer::generate(cref<AccelerationStageInput> asi_) const {

    const bool isFwd = asi_.transferType == AccelerationStageTransferType::eForward;
    const bool isDyn = asi_.bindingMode == DataBindingMode::eDynamic;

    if (isFwd) {
        return generateToken(
            _prefix,
            { _inScope },
            asi_.token.value
        );
    }

    if (not isFwd && isDyn) {
        return generateToken(
            _prefix,
            { _bindScope },
            asi_.token.value
        );
    }

    if (not isFwd && not isDyn) {
        return generateToken(
            no_prefix,
            { _bindScope },
            asi_.token.value
        );
    }

    return Token {};
}

Token Tokenizer::generate(cref<AccelerationStageOutput> aso_) const {

    const bool isFwd = aso_.transferType == AccelerationStageTransferType::eForward;
    const bool isDyn = aso_.bindingMode == DataBindingMode::eDynamic;

    if (isFwd) {
        return generateToken(
            _prefix,
            { _outScope },
            aso_.token.value
        );
    }

    if (not isFwd && isDyn) {
        return generateToken(
            _prefix,
            { _bindScope },
            aso_.token.value
        );
    }

    if (not isFwd && not isDyn) {
        return generateToken(
            no_prefix,
            { _bindScope },
            aso_.token.value
        );
    }

    return Token {};
}

Token Tokenizer::transformAccStageOut(cref<Token> src_, const bool forwarding, const bool dynamic) const {

    if (forwarding) {
        return generateToken(
            _prefix,
            { _outScope },
            src_.value
        );
    }

    if (not forwarding && dynamic) {
        return generateToken(
            _prefix,
            { _bindScope },
            src_.value
        );
    }

    if (not forwarding && not dynamic) {
        return generateToken(
            no_prefix,
            { _bindScope },
            src_.value
        );
    }

    return Token {};

}
