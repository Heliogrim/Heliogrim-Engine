#include "AssetImportTypeItem.hpp"

#include <Engine.Common/Make.hpp>
#include <Engine.Reflow/Style/BoundStyleSheet.hpp>

#include "Editor.UI/Color/Dark.hpp"

using namespace ember::editor::ui;
using namespace ember::engine::reflow;
using namespace ember;

AssetImportTypeItem::AssetImportTypeItem() :
    Button(
        BoundStyleSheet::make(
            StyleSheet {
                .minWidth = { true, ReflowUnit { ReflowUnitType::eAbsolute, 88.F } },
                .minHeight = { true, ReflowUnit { ReflowUnitType::eAbsolute, 124.F } },
                .padding = { true, Padding { 6.F } },
                .margin = { true, Margin { 4.F } },
                .reflowSpacing = { true, ReflowSpacing::eSpaceBetween },
                .borderRadius = { true, BorderRadius { 6.F } },
                .color = { true, color::Dark::backgroundInnerField },
            }
        )
    ) {}
