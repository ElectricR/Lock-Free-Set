#pragma once
#include <concepts>
#include <limits>

namespace detail {

template<class T>
concept HasMaxValue = requires { T::max(); };

template<class T>
concept SetApplicable = std::totally_ordered<T> && (HasMaxValue || std::is_numeric_v<T>);

template<SetApplicable T>
T get_max_value() {
    if constexpr (HasMaxValue<T>) {
        return T::max();
    }
    else {
        return std::numeric_limits<T>::max();
    }
}

struct Node;
union Operation;

struct InsertOperation {
    bool is_left;
    bool is_updated = false;
    Node *expected_node;
    Node *new_node;
};

struct RotateOperation {
    state;
    Node *parent;
    Node *node;
    Node *child;
    Operation *parent_op;
    Operation *node_op;
    Operation *child_op;
    bool is_right_rotation;
    bool dir;
};

union Operation {
    InsertOperation insert_op;
    RotateOperation rotate_op;
};

template<LockFreeApplicable T>
struct Node {
    T key;
    Node *left = nullptr;
    Node *right = nullptr;
    Operation *op = nullptr;
    unsigned local_height;
    unsigned left_height;
    unsigned right_height;
    bool deleted = false;
    bool removed = false;
};


template<LockFreeApplicable T>
class Set {
public:
    bool add(const T& value) {}
    bool remove(const T& value) {}
    bool contains(const T& value) {}
    bool isEmpty() {}
private:
    T root_ = detail::get_max_value<T>();
};
