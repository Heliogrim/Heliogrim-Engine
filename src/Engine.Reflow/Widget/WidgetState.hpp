#pragma once

namespace ember::engine::reflow {

    struct WidgetState {
        bool focus = false;
        bool hover = false;
        bool visible = true;

        /**/
        bool pending = true;
        /**/

        /*
        bool touched;
        bool deleted;
         */
    };
}
