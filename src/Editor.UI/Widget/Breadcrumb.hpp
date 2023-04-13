#pragma once

#include <Engine.Common/Collection/AssociativeKey.hpp>
#include <Engine.Filesystem/Url.hpp>
#include <Engine.Reflow/Widget/HorizontalPanel.hpp>

namespace hg::editor::ui {
    struct BreadcrumbEntry {
        AssocKey<string> key;
        wptr<engine::reflow::Widget> widget;
    };

    class Breadcrumb final :
        public engine::reflow::HorizontalPanel {
    public:
        using this_type = Breadcrumb;

        using action_fnc_type = _STD function<void(cref<fs::Url>)>;

    protected:
        Breadcrumb();

    public:
        ~Breadcrumb() override;

    private:
        Vector<BreadcrumbEntry> _entries;

    public:
        void addNavEntry(cref<AssocKey<string>> key_, cref<string> title_, cref<fs::Url> value_);

        void removeNavEntry(cref<AssocKey<string>> key_);

        void clearNavEntries();

    private:
        u64 _areg;
        Vector<_STD pair<u64, action_fnc_type>> _actions;

    private:
        void handleAction(cref<fs::Url> url_);

    public:
        u64 onAction(mref<action_fnc_type> fnc_);

        void offAction(u64 action_);

    public:
        [[nodiscard]] static sptr<Breadcrumb> make();
    };
}
