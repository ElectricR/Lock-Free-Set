#pragma once

#include <set>

#include <atomic>
#include <cds/init.h>
#include <cds/gc/hp.h>

namespace detail {

template<class T>
void deleter(void *set) {
    delete static_cast<std::set<T>*>(set);
}

} //namespace detail

template<class T>
class Set {
public:
    Set() {
        set_ = new std::set<T>{};
    }

    ~Set() {
        delete set_.load();
    }

    bool add(const T& key) {
        while (true) {
            cds::gc::HP::Guard guard;
            std::set<T>* current_set = guard.protect<std::set<T>*>(set_);
            std::set<T>* new_set = new std::set<T>(*current_set);
            if (new_set->contains(key)) {
                delete new_set;
                return false;
            }
            new_set->insert(key);
            if (set_.compare_exchange_weak(current_set, new_set)) {
                cds::gc::HP::retire(current_set, detail::deleter<T>);
                return true;
            }
            delete new_set;
        }
    }

    bool remove(const T& key) {
        while (true) {
            cds::gc::HP::Guard guard;
            std::set<T>* current_set = guard.protect<std::set<T>*>(set_);
            std::set<T>* new_set = new std::set<T>(*current_set);
            if (!new_set->contains(key)) {
                delete new_set;
                return false;
            }
            new_set->erase(key);
            if (set_.compare_exchange_weak(current_set, new_set)) {
                cds::gc::HP::retire(current_set, detail::deleter<T>);
                return true;
            }
            delete new_set;
        }
    }

    bool contains(const T& key) const {
        cds::gc::HP::Guard guard;
        std::set<T>* current_set = guard.protect<std::set<T>*>(set_);
        return current_set->contains(key);
    }

    bool isEmpty() const {
        cds::gc::HP::Guard guard;
        std::set<T>* current_set = guard.protect<std::set<T>*>(set_);
        return current_set->empty();
    }

private:
    std::atomic<std::set<T>*> set_;
};
