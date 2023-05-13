#include "AssetImportTypeItem.hpp"

using namespace hg::editor::ui;
using namespace hg::engine::reflow;
using namespace hg;

AssetImportTypeItem::AssetImportTypeItem() :
    Button() {
    /**/
    attributes().minWidth.setValue({ ReflowUnitType::eAbsolute, 88.F });
    attributes().minHeight.setValue({ ReflowUnitType::eAbsolute, 124.F });
    attributes().padding.setValue(Padding { 6.F });
    attributes().justify.setValue(ReflowSpacing::eSpaceBetween);
}
