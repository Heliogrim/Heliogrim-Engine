#pragma once

#include <Engine.Core/Module/DependencyKey.hpp>
#include <Engine.Common/StringLiterals.hpp>

namespace hg::editor {
    inline constexpr static engine::core::DependencyKey EditorDepKey { "::hg::editor::Editor"sv };
}
