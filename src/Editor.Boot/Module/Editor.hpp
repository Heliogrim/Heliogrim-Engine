#pragma once

#include <Engine.Common/StringLiterals.hpp>
#include <Engine.Core/Module/DependencyKey.hpp>

namespace hg::editor {
    inline constexpr static engine::core::DependencyKey EditorDepKey { "::hg::editor::Editor"sv };
}
