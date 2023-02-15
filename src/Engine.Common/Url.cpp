#include "Url.hpp"

#include <sstream>

using namespace ember;

string Url::encode() const noexcept {

    _STD stringstream ss {};
    ss << _scheme << "://"sv;
    // TODO: user, password, host, port
    ss << _path;

    return ss.str();
}
