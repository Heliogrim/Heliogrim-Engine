#include "Url.hpp"

using namespace hg::engine::fs;
using namespace hg;

Url::Url() :
    _scheme(),
    _path() {}

Url::Url(mref<Path> path_) :
    _scheme(),
    _path(std::move(path_)) {}

Url::Url(string_view scheme_, mref<Path> path_) :
    _scheme(scheme_),
    _path(std::move(path_)) {}

Url::~Url() = default;

bool Url::hasScheme() const noexcept {
    return not _scheme.empty();
}

const string_view Url::scheme() const noexcept {
    return _scheme;
}

ref<Url::this_type> Url::setScheme(string_view scheme_) {
    _scheme = scheme_;
    return *this;
}

bool Url::hasPath() const noexcept {
    return not _path.empty();
}

cref<Path> Url::path() const noexcept {
    return _path;
}

ref<Url::this_type> Url::setPath(mref<Path> path_) {
    _path = std::move(path_);
    return *this;
}

bool Url::empty() const noexcept {
    return _scheme.empty() && _path.empty();
}

string Url::encode() const noexcept {
    if (not _scheme.empty()) {
        return _scheme + "://" + _path.string();
    }
    return _path.string();
}
