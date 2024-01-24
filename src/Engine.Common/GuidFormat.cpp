#include "GuidFormat.hpp"

#include <sstream>

using namespace hg;

static constexpr char delimiter = '-';

string hg::encodeGuid4228(cref<Guid> guid_) {
    std::stringstream ss {};
    ss << guid_.pre << delimiter << guid_.c0 << delimiter << guid_.c1 << delimiter << guid_.post;
    return ss.str();
}

Guid hg::decodeGuid4228(string_view str_) {

    Guid tmp {};

    const auto c0Idx = str_.find_first_of(delimiter, 1) + 1;
    const auto c1Idx = str_.find_first_of(delimiter, c0Idx) + 1;
    const auto postIdx = str_.find_first_of(delimiter, c1Idx) + 1;

    tmp.pre = std::stoul(string { str_.substr(0, c0Idx - 0 - 1) });
    tmp.c0 = std::stoi(string { str_.substr(c0Idx, c1Idx - c0Idx - 1) });
    tmp.c1 = std::stoi(string { str_.substr(c1Idx, postIdx - c1Idx - 1) });
    tmp.post = std::stoull(string { str_.substr(postIdx, string_view::npos) });

    return tmp;
}
