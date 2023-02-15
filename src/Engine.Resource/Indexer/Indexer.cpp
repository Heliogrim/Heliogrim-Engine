#include "Indexer.hpp"

#include <Engine.Common/Collection/Queue.hpp>

using namespace ember::engine::res;
using namespace ember;

Indexer::Indexer() noexcept {}

Indexer::~Indexer() noexcept {
    tidy();
}

void Indexer::tidy() {
    for (auto entry : _watcher) {
        delete entry;
    }
}

void Indexer::addRoot(cref<fs::File> root_) noexcept {

    const auto found = _STD find_if(
        _roots.begin(),
        _roots.end(),
        [&root_](cref<fs::File> entry_) {
            return root_ == entry_;
        }
    );

    if (found != _roots.end()) {
        return;
    }

    _roots.push_back(root_);
}

bool Indexer::removeRoot(cref<fs::File> root_) noexcept {

    const auto where = _STD find_if(
        _roots.begin(),
        _roots.end(),
        [&root_](cref<fs::File> entry_) {
            return root_ == entry_;
        }
    );

    if (where != _roots.end()) {
        return false;
    }

    _roots.erase(where);
    return true;
}

void Indexer::scan(cref<fs::File> root_) {

    queue<fs::File> backlog {};
    backlog.push(root_);

    while (!backlog.empty()) {

        const fs::File entry = backlog.front();

        if (entry.isDirectory()) {
            /**
             * If current node is directory, push every subnode to backlog
             */
            for (auto file : entry.files()) {
                backlog.push(file);
            }
        } else {
            /**
             * If current node is file, index current node
             */
            reindex(entry);
        }

        backlog.pop();
    }
}

void Indexer::scan() {

    for (const auto& entry : _roots) {
        scan(entry);
    }
}

void Indexer::watch(cref<fs::File> root_) {

    auto watcher = new Watcher {
        root_
    };

    watcher->setCreateCallback(
        [this](auto param) {
            this->reindex(param);
        }
    );

    watcher->setModifiedCallback(
        [this](auto param) {
            this->reindex(param);
        }
    );

    watcher->setEraseCallback(
        [this](auto param) {
            this->removeIndex(param);
        }
    );

    _watcher.push_back(
        new Watcher {
            root_
        }
    );
}

void Indexer::watch() {

    for (const auto& entry : _roots) {
        watch(entry);
    }
}

void Indexer::on(cref<std::function<bool(cref<fs::File> file_)>> callback_) {
    _callbacks.push_back(callback_);
}

void Indexer::on(mref<std::function<bool(cref<fs::File> file_)>> callback_) {
    _callbacks.push_back(_STD move(callback_));
}

void Indexer::reindex(cref<fs::File> file_) {

    if (_STD ranges::any_of(
        _callbacks.begin(),
        _callbacks.end(),
        [&file_](const auto& entry) {
            return entry(file_);
        }
    )) {
        _ack.insert(file_);
    }
}

void Indexer::removeIndex(cref<fs::File> file_) {}
