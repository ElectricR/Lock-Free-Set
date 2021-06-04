#pragma once
#include <atomic>
#include <optional>
#include <utility>
#include <concepts>
#include <limits>
#include "flags.h"
#include <cds/init.h>
#include <cds/gc/hp.h>

namespace detail {

template<class T>
concept SetApplicable = true;

template<SetApplicable T>
struct Node {
    std::optional<T> key;
    std::atomic<Node<T>*> next = nullptr;
};

template<SetApplicable T>
void deleter(void *t) {
    delete static_cast<T*>(t);
}

} //namespace detail

template<detail::SetApplicable T>
class Set {
public:

    Set() {
        root_ = new detail::Node<T>();
    }

    ~Set() {
        delete root_.load();
    }

    bool add(const T& key) {
        detail::Node<T> *new_node = new detail::Node<T>();
        new_node->key = key;
        while (true) {
            auto [prev_guard, curr_guard] = this->find(key);
            detail::Node<T> *prev = prev_guard.template get<detail::Node<T>>();
            detail::Node<T> *curr = curr_guard.template get<detail::Node<T>>();
            if (curr && curr->key == key) {
                delete new_node;
                return false;
            }
            else {
                new_node->next = curr;
                if (prev->next.compare_exchange_weak(curr, new_node)) {
                    return true;
                }
            }
        }
    }
    
    bool remove(const T& key) {
        while (true) {
            auto [prev_guard, curr_guard] = this->find(key);
            detail::Node<T> *prev = prev_guard.template get<detail::Node<T>>();
            detail::Node<T> *curr = curr_guard.template get<detail::Node<T>>();
            if (!curr || curr->key != key) {
                return false;
            }
            else {
                detail::Node<T>* next = GET_POINTER(detail::Node<T>*, curr->next.load());
                detail::Node<T>* flagged = next;
                SET_FLAG(detail::Node<T>*, flagged, true); 
                if (!curr->next.compare_exchange_weak(next, flagged)) {
                    continue;
                }
                if (prev->next.compare_exchange_weak(curr, next)) {
                    cds::gc::HP::retire(curr, detail::deleter<T>);
                }
                return true;
            }
        }
    }

    bool contains(const T& key) {
        auto [prev_guard, curr_guard] = this->find(key);
        detail::Node<T> *curr = curr_guard.template get<detail::Node<T>>();
        if (curr && curr->key == key) {
            return true;
        }
        else {
            return false;
        }
    }

    bool isEmpty() {
        return root_.load()->next == nullptr;
    }

private:
    std::pair<cds::gc::HP::Guard, cds::gc::HP::Guard> find(const T& key) {
        while (true) {
            cds::gc::HP::Guard prev_guard;
            cds::gc::HP::Guard curr_guard;
            cds::gc::HP::Guard next_guard;
            detail::Node<T>* prev = prev_guard.protect<detail::Node<T>*>(root_);
            detail::Node<T>* curr = curr_guard.protect<detail::Node<T>*>(prev->next);
            detail::Node<T>* next = nullptr;
            while (true) {
                if (!curr) {
                    return std::pair<cds::gc::HP::Guard, cds::gc::HP::Guard>(std::move(prev_guard), std::move(curr_guard));
                }
                next = next_guard.protect<detail::Node<T>*>(curr->next);
                bool flag = GET_FLAG(next); 
                next = GET_POINTER(detail::Node<T>*, next);
                if (flag) {
                    if (!prev->next.compare_exchange_weak(curr, next)) {
                        break;
                    }
                    cds::gc::HP::retire(curr, detail::deleter<T>);
                    curr = curr_guard.assign<detail::Node<T>>(next);
                }
                else {
                    if (curr->key >= key) {
                        return std::pair<cds::gc::HP::Guard, cds::gc::HP::Guard>(std::move(prev_guard), std::move(curr_guard));
                    }
                    prev_guard = std::move(curr_guard);
                    curr_guard = std::move(next_guard);
                    prev = prev_guard.get<detail::Node<T>>();
                    curr = curr_guard.get<detail::Node<T>>();
                }
            }
        }
    }

    std::atomic<detail::Node<T>*> root_;
};
