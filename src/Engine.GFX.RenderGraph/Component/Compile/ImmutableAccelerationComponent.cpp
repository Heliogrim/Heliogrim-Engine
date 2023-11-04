#include "ImmutableAccelerationComponent.hpp"

#include "../Runtime/ImmutableAccelerationComponent.hpp"

#include <Engine.Pedantic/Clone/Clone.hpp>
#include <Engine.Reflect/Cast.hpp>

#include "../../Relation/TextureDescription.hpp"

using namespace hg::engine::gfx::render::graph;
using namespace hg;

bool CompileImmutableAccelerationComponent::validateInputSymbol(nmpt<const acc::Symbol> symbol_) const noexcept {
    const auto& description = symbol_->description;
    const auto texture = Cast<TextureDescription>(description.get());
    return texture;
}

bool CompileImmutableAccelerationComponent::validateTransformSymbol(
    nmpt<const acc::Symbol> inputSymbol_,
    nmpt<const acc::Symbol> outputSymbol_
) const noexcept {

    if (not validateInputSymbol(inputSymbol_)) {
        return false;
    }

    if (not validateOutputSymbol(outputSymbol_)) {
        return false;
    }

    /**/

    const auto inputTexture = Cast<TextureDescription>(inputSymbol_->description.get());
    const auto outputTexture = Cast<TextureDescription>(outputSymbol_->description.get());

    return inputTexture->isCompatible(outputTexture) && outputTexture->isCompatible(inputTexture);
}

bool CompileImmutableAccelerationComponent::validateOutputSymbol(nmpt<const acc::Symbol> symbol_) const noexcept {
    const auto& description = symbol_->description;
    const auto texture = Cast<TextureDescription>(description.get());
    return texture;
}

void CompileImmutableAccelerationComponent::addInput(mref<smr<const acc::Symbol>> symbol_) {

    if (_STD ranges::find(_passInputSymbols, symbol_) != _passInputSymbols.end()) {
        __debugbreak();
        return;
    }

    /**/

    bool valid = false;
    if (const auto outIt = _STD ranges::find(_passOutputSymbols, symbol_); outIt != _passOutputSymbols.end()) {
        valid = validateTransformSymbol(symbol_.get(), outIt->get());
    } else {
        valid = validateInputSymbol(symbol_.get());
    }

    /**/

    if (not valid) {
        __debugbreak();
        return;
    }

    _passInputSymbols.push_back(_STD move(symbol_));
}

void CompileImmutableAccelerationComponent::addTransform(mref<smr<const acc::Symbol>> symbol_) {

    const bool inputStored = _STD ranges::find(_passInputSymbols, symbol_) != _passInputSymbols.end();
    const bool outputStored = _STD ranges::find(_passOutputSymbols, symbol_) != _passOutputSymbols.end();

    if (inputStored && outputStored) {
        __debugbreak();
        return;
    }

    /**/

    if (not validateTransformSymbol(symbol_.get(), symbol_.get())) {
        __debugbreak();
        return;
    }

    if (not inputStored) {
        _passInputSymbols.push_back(clone(symbol_));
    }

    if (not outputStored) {
        _passOutputSymbols.push_back(_STD move(symbol_));
    }
}

void CompileImmutableAccelerationComponent::addOutput(mref<smr<const acc::Symbol>> symbol_) {

    if (_STD ranges::find(_passOutputSymbols, symbol_) != _passOutputSymbols.end()) {
        __debugbreak();
        return;
    }

    /**/

    bool valid = false;
    if (const auto inIt = _STD ranges::find(_passInputSymbols, symbol_); inIt != _passInputSymbols.end()) {
        valid = validateTransformSymbol(inIt->get(), symbol_.get());
    } else {
        valid = validateOutputSymbol(symbol_.get());
    }

    /**/

    if (not valid) {
        __debugbreak();
        return;
    }

    _passOutputSymbols.push_back(_STD move(symbol_));
}

uptr<AccelerationComponent> CompileImmutableAccelerationComponent::
compile(cref<CompilePassContext> ctx_) const noexcept {
    return ctx_.getGraphNodeAllocator()->allocate<ImmutableAccelerationComponent>(
        clone(_passOutputSymbols),
        clone(_passInputSymbols)
    );
}
