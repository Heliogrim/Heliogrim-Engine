#pragma once
#include <Engine.Common/Collection/InlineAutoArray.hpp>

#include "Dialect.hpp"
#include "Lang/TextBlock.hpp"

namespace hg::engine::accel::lang {
    class Language {
    public:
        constexpr Language() noexcept = default;

        Language(cref<Language> other_) :
            dialect(other_.dialect),
            text() {
            for (const auto& val : other_.text) {
                text.emplace_back(val);
            }
        }

        Language(mref<Language> other_) noexcept = default;

        Dialect dialect;
        InlineAutoArray<TextBlock, 1uLL> text;
    };
}
