#pragma once

#if not defined(HG_MODULE_INTERFACE_UNIT)
import Engine.Reflow;
import Engine.Resource;
#endif

namespace hg::engine::graphics {
    typedef resource::Resource<reflow::Font> FontResource;
}
