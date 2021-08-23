#include "CameraFrustum.hpp"

#include <Engine.Common/Types.hpp>

using namespace ember::engine::gfx;
using namespace ember;

void CameraFrustum::update(math::mat4 src_) {
    _planes[eLeft].x = src_[0].w + src_[0].x;
    _planes[eLeft].y = src_[1].w + src_[1].x;
    _planes[eLeft].z = src_[2].w + src_[2].x;
    _planes[eLeft].w = src_[3].w + src_[3].x;

    _planes[eRight].x = src_[0].w - src_[0].x;
    _planes[eRight].y = src_[1].w - src_[1].x;
    _planes[eRight].z = src_[2].w - src_[2].x;
    _planes[eRight].w = src_[3].w - src_[3].x;

    _planes[eTop].x = src_[0].w + src_[0].y;
    _planes[eTop].y = src_[1].w + src_[1].y;
    _planes[eTop].z = src_[2].w + src_[2].y;
    _planes[eTop].w = src_[3].w + src_[3].y;

    _planes[eBottom].x = src_[0].w - src_[0].y;
    _planes[eBottom].y = src_[1].w - src_[1].y;
    _planes[eBottom].z = src_[2].w - src_[2].y;
    _planes[eBottom].w = src_[3].w - src_[3].y;

    _planes[eBack].x = src_[0].w + src_[0].z;
    _planes[eBack].y = src_[1].w + src_[1].z;
    _planes[eBack].z = src_[2].w + src_[2].z;
    _planes[eBack].w = src_[3].w + src_[3].z;

    _planes[eFront].x = src_[0].w - src_[0].z;
    _planes[eFront].y = src_[1].w - src_[1].z;
    _planes[eFront].z = src_[2].w - src_[2].z;
    _planes[eFront].w = src_[3].w - src_[3].z;

    for (u8 i = 0; i < _planes.size(); i++) {
        const float length = std::sqrtf(
            _planes[i].x * _planes[i].x +
            _planes[i].y * _planes[i].y +
            _planes[i].z * _planes[i].z
        );
        _planes[i] /= length;
    }
}

bool CameraFrustum::checkSpherical(const math::vec3& origin_, const float radius_) const noexcept {
    for (u8 i = 0; i < _planes.size(); ++i) {
        if ((_planes[i].x * origin_.x) + (_planes[i].y * origin_.y) + (_planes[i].z * origin_.z) + _planes[i].w <= -
            radius_) {
            return false;
        }
    }

    return true;
}
