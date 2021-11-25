#pragma once

namespace ember::engine::proxy {

    struct proxy_category_tag {};

    struct proxy_gfx_category_tag : proxy_category_tag {};

    struct proxy_sfx_category_tag : proxy_category_tag {};

    struct proxy_pfx_category_tag : proxy_category_tag {};

    struct proxy_net_category_tag : proxy_category_tag {};

}
