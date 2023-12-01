#pragma once

namespace hg::engine::accel::lang {
    enum class AnnotationType : u8 {
        eUnknown = 0x0,
        eReadonly,
        eWriteonly,
        eForwardLinkage,
        eExternalLinkage,
        /**/
        eVkBindLocation
    };

    struct VkBindLocationAnnotation {
        s32 set;
        s32 location;
    };

    struct Annotation {
        AnnotationType type;

        union {
            VkBindLocationAnnotation vkBindLocation;
        };

        uptr<Annotation> next;
    };
}
