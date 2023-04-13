#include "AssetImportTypeItem.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

AssetImportTypeItem::AssetImportTypeItem() :
    Button() {
    /**/
    attr.minWidth.setValue({ ReflowUnitType::eAbsolute, 88.F });
    attr.minHeight.setValue({ ReflowUnitType::eAbsolute, 124.F });
    attr.padding.setValue(Padding { 6.F });
    attr.justify.setValue(ReflowSpacing::eSpaceBetween);
}
