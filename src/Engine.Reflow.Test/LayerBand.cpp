#include "pch.h"

/**/

import Engine.Common;
import Engine.Reflow;
import Engine.Reflow.Uikit;

using namespace hg::engine::reflow;
using namespace hg;

namespace ReflowModule {
	static sptr<Host> makeHost() {
		const sptr<Widget> content = make_sptr<uikit::VerticalLayout>();
		return make_sptr<Host>(content);
	}

	TEST(LayerBand, HostsOrderedByBand) {
		const auto window = make_sptr<Window>();
		window->setClientSize({ 800.F, 600.F });

		const auto tooltip = makeHost();
		const auto popupA = makeHost();
		const auto dialog = makeHost();
		const auto popupB = makeHost();

		discard = window->requestLayer(LayerBand::eTooltip, tooltip);
		discard = window->requestLayer(LayerBand::ePopup, popupA);
		discard = window->requestLayer(LayerBand::eDialog, dialog);
		discard = window->requestLayer(LayerBand::ePopup, popupB);

		const auto layers = window->layers();
		ASSERT_EQ(layers.size(), 4uLL);
		EXPECT_EQ(layers[0]->band, LayerBand::ePopup);
		EXPECT_EQ(layers[1]->band, LayerBand::ePopup);
		EXPECT_EQ(layers[2]->band, LayerBand::eDialog);
		EXPECT_EQ(layers[3]->band, LayerBand::eTooltip);
		EXPECT_EQ(layers[0]->host.lock(), popupA);
		EXPECT_EQ(layers[1]->host.lock(), popupB);

		const auto& children = *window->children();
		ASSERT_EQ(children.size(), 6uLL);
		EXPECT_EQ(children[2], popupA);
		EXPECT_EQ(children[3], popupB);
		EXPECT_EQ(children[4], dialog);
		EXPECT_EQ(children[5], tooltip);
	}

	TEST(LayerBand, DropKeepsOrder) {
		const auto window = make_sptr<Window>();
		window->setClientSize({ 800.F, 600.F });

		const auto popupA = makeHost();
		const auto dialog = makeHost();
		const auto popupB = makeHost();

		const auto layerA = window->requestLayer(LayerBand::ePopup, popupA);
		discard = window->requestLayer(LayerBand::eDialog, dialog);
		discard = window->requestLayer(LayerBand::ePopup, popupB);

		window->dropLayer(layerA);

		const auto layers = window->layers();
		ASSERT_EQ(layers.size(), 2uLL);
		EXPECT_EQ(layers[0]->host.lock(), popupB);
		EXPECT_EQ(layers[1]->host.lock(), dialog);
		EXPECT_EQ(window->children()->size(), 4uLL);
		EXPECT_EQ((*window->children())[2], popupB);
	}

	TEST(LayerBand, TooltipNotHitTestable) {
		EXPECT_FALSE(isHitTestable(LayerBand::eTooltip));
		EXPECT_FALSE(isHitTestable(LayerBand::eDragPreview));
		EXPECT_TRUE(isHitTestable(LayerBand::ePopup));
		EXPECT_TRUE(isHitTestable(LayerBand::eDialog));
		EXPECT_TRUE(isHitTestable(LayerBand::eContent));
	}
}
