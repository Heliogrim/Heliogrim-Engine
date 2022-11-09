#pragma once

#include <Engine.Common/Url.hpp>
#include <Engine.Reflow/Widget/HBox.hpp>

namespace ember::editor::ui {

    struct BreadcrumbEntry {
        AssocKey<string> key;
        wptr<engine::reflow::Widget> widget;
    };

    class Breadcrumb final :
        public engine::reflow::HBox {
    public:
        using this_type = Breadcrumb;

        using action_fnc_type = _STD function<void(cref<Url>)>;

    protected:
        Breadcrumb();

    public:
        ~Breadcrumb() override;

    private:
        Vector<BreadcrumbEntry> _entries;

    public:
        void addNavEntry(cref<AssocKey<string>> key_, cref<string> title_, cref<Url> value_);

        void removeNavEntry(cref<AssocKey<string>> key_);

        void clearNavEntries();

    private:
        u64 _areg;
        Vector<_STD pair<u64, action_fnc_type>> _actions;

    private:
        void handleAction(cref<Url> url_);

    public:
        u64 onAction(mref<action_fnc_type> fnc_);

        void offAction(u64 action_);

    public:
        [[nodiscard]] static sptr<Breadcrumb> make();
    };
}
