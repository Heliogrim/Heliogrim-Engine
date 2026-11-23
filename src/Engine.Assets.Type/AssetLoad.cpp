#include "AssetLoad.hpp"

#include <Engine.Common/GuidFormat.hpp>
#include <Engine.Serialization/Diagnostic/Codes.hpp>

using namespace hg::engine::assets;
using namespace hg;

Result<void, engine::diag::Diagnostics> engine::assets::checkStoreAllowed(std::span<const nmpt<const Asset>> assets_) {
	diag::Diagnostic refusal = diag::Diagnostic::error(
		serialization::codes::ser("save-refused"),
		"asset holds data this build did not understand; storing would drop it"
	);

	bool refused = false;
	for (const auto asset : assets_) {
		if (asset != nullptr && asset->carriesUnknownData()) {
			refused = true;
			refusal.addNote("asset " + encodeGuid4228(asset->getAssetGuid()) + ": fields were not understood on load");
		}
	}
	if (not refused) {
		return {};
	}

	diag::Diagnostics out {};
	out.emplace_back(::hg::move(refusal));
	return Unexpected<diag::Diagnostics> { ::hg::move(out) };
}
