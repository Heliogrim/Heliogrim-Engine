#include "pch.h"

/**/

import Engine.Common;
import Engine.Reflow;
import Engine.Reflow.Uikit;

using namespace hg::engine::reflow;
using namespace hg;

namespace ReflowModule {
	TEST(EventReply, ImplicitFromResponse) {
		EventReply r = EventResponse::eConsumed;
		EXPECT_TRUE(r.isConsumed());
		EXPECT_FALSE(r.capturePointer);
	}

	TEST(EventReply, MergeKeepsMaxResponseAndAnyRequest) {
		const auto merged = merge(EventReply::handled().withCapture(), EventReply::consumed());
		EXPECT_EQ(merged.response, EventResponse::eConsumed);
		EXPECT_TRUE(merged.capturePointer);
		EXPECT_FALSE(merged.releaseCapture);

		const auto a = make_sptr<uikit::VerticalLayout>();
		const auto b = make_sptr<uikit::VerticalLayout>();
		const auto later = merge(EventReply::handled().withFocus(a), EventReply::handled().withFocus(b));
		EXPECT_EQ(later.focus.lock(), b);
		const auto kept = merge(EventReply::handled().withFocus(a), EventReply::handled());
		EXPECT_EQ(kept.focus.lock(), a);
	}
}
