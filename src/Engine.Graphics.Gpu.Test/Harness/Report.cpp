#include "Report.hpp"

#include <algorithm>
#include <format>
#include <fstream>
#include <sstream>
#include <ryml/ryml.hpp>
#include <ryml/ryml_std.hpp>

#ifdef WIN32
#include <Engine.Common/stdafx.h>
/**/
#include <libloaderapi.h>
#endif

using namespace hg;
using namespace hg::test::gpu;

namespace {
	inline constexpr static u32 channelCount = 4;

	[[nodiscard]] f32 texelError(ref<const math::fvec4> a_, ref<const math::fvec4> b_);

	[[nodiscard]] ErrorSummary summarize(ref<const Raster> a_, ref<const Raster> b_);

	[[nodiscard]] String summaryText(ref<const ErrorSummary> s_);

	[[nodiscard]] String fileName(const StringView test_, const StringView suffix_);
}

bool Report::passed() const noexcept {
	return std::ranges::all_of(checks, [](ref<const CheckResult> c_) { return c_.passed; });
}

String Report::text() const {
	std::stringstream out;
	out << notes;
	for (const auto& c : checks) {
		out << (c.passed ? "PASS " : "FAIL ") << c.name << ": failing " << c.failing << ", measured " << c.measured << '\n';
	}
	out << "vs expected: " << (errorMap.extent == extent ? summaryText(vsExpected) : String { "n/a (extent differs)" }) << '\n';
	if (vsPrevious.has_value()) {
		out << "vs previous: " << summaryText(*vsPrevious) << ", " << changedTexels << " texels changed\n";
	} else {
		out << "vs previous: no stored run\n";
	}

	for (const auto& [at, description] : inputs) {
		out << '(' << at.x << ", " << at.y << ") " << description << '\n';
	}
	return out.str();
}

Report hg::test::gpu::makeReport(
	const StringView test_,
	ref<const Raster> actual_,
	ref<const Raster> expected_,
	Vector<CheckResult> checks_,
	ref<const InputsAt> inputs_,
	const ptr<const Raster> previous_
) {
	Report report {
		String { test_ },
		actual_.extent,
		std::move(checks_),
		ErrorSummary { 0.F, math::uivec2 { 0, 0 }, 0.F },
		Raster { math::uivec2 { 0, 0 }, {} },
		tl::nullopt,
		0u,
		{},
		{}
	};

	// The extent check result already fails the report; nothing below may index past the smaller raster.
	if (actual_.extent == expected_.extent) {
		report.vsExpected = summarize(actual_, expected_);
		report.errorMap = Raster { actual_.extent, Vector<math::fvec4>(actual_.texels.size()) };
		for (size_t i = 0; i < actual_.texels.size(); ++i) {
			for (u32 c = 0; c < channelCount; ++c) {
				report.errorMap.texels[i][c] = std::abs(actual_.texels[i][c] - expected_.texels[i][c]);
			}
		}
	}

	Vector<math::uivec2> listed;
	const auto list = [&](const math::uivec2 at_) {
		if (listed.size() >= listCap) {
			return;
		}

		if (std::ranges::find(listed, at_) == listed.end()) {
			listed.push_back(at_);
		}
	};
	for (const auto& c : report.checks) {
		std::ranges::for_each(c.failingTexels, list);
	}

	if (previous_ != nullptr && previous_->extent == actual_.extent) {
		report.vsPrevious = SomeVal<ErrorSummary>(summarize(actual_, *previous_));
		for (u32 y = 0; y < actual_.extent.y; ++y) {
			for (u32 x = 0; x < actual_.extent.x; ++x) {
				if (texelError(actual_.at(x, y), previous_->at(x, y)) > 0.F) {
					++report.changedTexels;
					list(math::uivec2 { x, y });
				}
			}
		}
	}

	for (const auto& at : listed) {
		if (report.inputs.size() >= listCap) {
			break;
		}

		report.inputs.emplace_back(at, inputs_ ? inputs_(at.x, at.y) : String {});
	}
	return report;
}

std::filesystem::path store::resultDir() {
	std::filesystem::path base;
	#ifdef WIN32
	std::wstring buffer(MAX_PATH, L'\0');
	while (true) {
		const auto length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
		if (length < buffer.size()) {
			buffer.resize(length);
			break;
		}

		buffer.resize(buffer.size() * 2);
	}

	base = std::filesystem::path { buffer }.parent_path();
	#else
	base = std::filesystem::current_path();
	#endif
	auto dir = base / "gpu-results";
	std::filesystem::create_directories(dir);
	return dir;
}

Opt<Raster> store::loadPrevious(const StringView test_) {
	const auto path = resultDir() / fileName(test_, ".last.f32");
	std::error_code ec;
	const auto size = std::filesystem::file_size(path, ec);
	std::ifstream in { path, std::ios::binary };
	if (ec || not in) {
		return tl::nullopt;
	}

	u32 extent[2] {};
	in.read(reinterpret_cast<ptr<char>>(extent), sizeof(extent));
	if (not in) {
		return tl::nullopt;
	}

	// Compare texel counts instead of bytes so a corrupt header cannot overflow or request an absurd buffer.
	const auto payload = size - sizeof(extent);
	if (payload % sizeof(math::fvec4) != 0 || static_cast<u64>(extent[0]) * extent[1] != payload / sizeof(math::fvec4)) {
		return tl::nullopt;
	}

	Raster raster { math::uivec2 { extent[0], extent[1] }, Vector<math::fvec4>(static_cast<size_t>(extent[0]) * extent[1]) };
	in.read(reinterpret_cast<ptr<char>>(raster.texels.data()), static_cast<std::streamsize>(payload));
	if (not in) {
		return tl::nullopt;
	}

	return SomeVal<Raster>(std::move(raster));
}

void store::savePassing(const StringView test_, ref<const Raster> actual_) {
	const auto path = resultDir() / fileName(test_, ".last.f32");
	auto temp = path;
	temp += ".tmp";

	{
		std::ofstream out { temp, std::ios::binary | std::ios::trunc };
		const u32 extent[2] { actual_.extent.x, actual_.extent.y };
		out.write(reinterpret_cast<ptr<const char>>(extent), sizeof(extent));
		out.write(
			reinterpret_cast<ptr<const char>>(actual_.texels.data()),
			static_cast<std::streamsize>(actual_.texels.size() * sizeof(math::fvec4))
		);
		out.flush();

		if (not out) {
			out.close();
			std::error_code ec;
			std::filesystem::remove(temp, ec);
			return;
		}
	}

	std::error_code ec;
	std::filesystem::rename(temp, path, ec);
	if (ec) {
		std::filesystem::remove(temp, ec);
	}
}

void store::writeReport(ref<const Report> report_) {
	ryml::Tree tree;
	auto root = tree.rootref();
	root |= ryml::MAP;

	const auto summary = [&](ryml::NodeRef parent_, const ptr<const char> key_, ref<const ErrorSummary> s_) {
		auto node = parent_[ryml::to_csubstr(key_)];
		node |= ryml::MAP;
		node["max"] << s_.maxError;
		node["x"] << s_.maxAt.x;
		node["y"] << s_.maxAt.y;
		node["mean"] << s_.meanError;
	};

	root["test"] << report_.test;
	root["passed"] << report_.passed();
	root["width"] << report_.extent.x;
	root["height"] << report_.extent.y;

	/**/

	auto checks = root["checks"];
	checks |= ryml::SEQ;
	for (const auto& c : report_.checks) {
		auto node = checks.append_child();
		node |= ryml::MAP;
		node["name"] << c.name;
		node["passed"] << c.passed;
		node["failing"] << c.failing;
		node["measured"] << c.measured;
	}

	summary(root, "vsExpected", report_.vsExpected);
	if (report_.vsPrevious.has_value()) {
		summary(root, "vsPrevious", *report_.vsPrevious);
		root["changedTexels"] << report_.changedTexels;
	}

	auto inputs = root["inputs"];
	inputs |= ryml::SEQ;
	for (const auto& [at, description] : report_.inputs) {
		auto node = inputs.append_child();
		node |= ryml::MAP;
		node["x"] << at.x;
		node["y"] << at.y;
		node["inputs"] << description;
	}

	// One row of floats per raster row, using the maximum channel error of each texel.
	auto map = root["errorMap"];
	map |= ryml::SEQ;
	for (u32 y = 0; y < report_.errorMap.extent.y; ++y) {
		auto row = map.append_child();
		row |= ryml::SEQ | ryml::FLOW_SL;
		for (u32 x = 0; x < report_.errorMap.extent.x; ++x) {
			row.append_child() << texelError(report_.errorMap.at(x, y), math::fvec4 { 0.F });
		}
	}

	/**/

	// Write a temporary file and rename it, like `savePassing`, so a failed write never leaves a truncated report.
	const auto path = resultDir() / fileName(report_.test, ".report.yaml");
	auto temp = path;
	temp += ".tmp";

	{
		std::ofstream out { temp, std::ios::trunc };
		out << tree;
		out.flush();
		if (not out) {
			out.close();
			std::error_code ec;
			std::filesystem::remove(temp, ec);
			return;
		}
	}

	std::error_code ec;
	std::filesystem::rename(temp, path, ec);
	if (ec) {
		std::filesystem::remove(temp, ec);
	}
}

/**/

namespace {
	[[nodiscard]] f32 texelError(ref<const math::fvec4> a_, ref<const math::fvec4> b_) {
		f32 error = 0.F;
		for (u32 c = 0; c < channelCount; ++c) {
			error = std::max(error, std::abs(a_[c] - b_[c]));
		}
		return error;
	}

	[[nodiscard]] ErrorSummary summarize(ref<const Raster> a_, ref<const Raster> b_) {
		ErrorSummary summary { 0.F, math::uivec2 { 0, 0 }, 0.F };
		f64 sum = 0.0;
		for (u32 y = 0; y < a_.extent.y; ++y) {
			for (u32 x = 0; x < a_.extent.x; ++x) {
				const auto error = texelError(a_.at(x, y), b_.at(x, y));
				sum += static_cast<f64>(error);
				if (error > summary.maxError) {
					summary.maxError = error;
					summary.maxAt = math::uivec2 { x, y };
				}
			}
		}

		const auto count = static_cast<f64>(a_.texels.size());
		summary.meanError = count > 0.0 ? static_cast<f32>(sum / count) : 0.F;
		return summary;
	}

	[[nodiscard]] String summaryText(ref<const ErrorSummary> s_) {
		return std::format("max {:g} at ({}, {}), mean {:g}", s_.maxError, s_.maxAt.x, s_.maxAt.y, s_.meanError);
	}

	[[nodiscard]] String fileName(const StringView test_, const StringView suffix_) {
		return String { test_ } + String { suffix_ };
	}
}
