#pragma once
#include <concepts>
#include <limits>

#define GET_FLAG(p) static_cast<OperationFlag>((reinterpret_cast<long>(p) & 0x0003000000000000) >> 48)
#define GET_POINTER(type, p) reinterpret_cast<type>(reinterpret_cast<long>(p) & 0x0000ffffffffffff)
#define SET_FLAG(type, p, flag) p = reinterpret_cast<type>(reinterpret_cast<long>(p) | (static_cast<long>(flag) << 48))

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


enum class OperationFlag : short {
    NONE,
    MARK,
    ROTATE,
    INSERT
};


template<LockFreeApplicable T>
class Set {
public:
    bool add(const T& value) {}
    bool remove(const T& value) {}
    bool contains(const T& value) {
        Node *node = &root_;
        Node *next = node->left;
        Operation *node_op = node->op;
        bool result = false;
        T* node_key = &node->key;
        while (next != nullptr) {
            node = next;
            node_op = node->op;
            node_key = node->key;
            if (value < node_key) {
                next = node->left;
            }
            else if (value > node_key) {
                next = node->right;
            }
            else {
                result = true;
                break;
            }
        }
        if (result && node->deleted) {
            if (GET_FLAG(node->op) == OperationFlag::INSERT) {
                if (GET_POINTER(Operation*, node_op)->insert_op.new_node->key == value) {
                    return true;
                }
            }
            return false;
        }
        return result;
    }
    bool isEmpty() {}
private:
    T root_ = detail::get_max_value<T>();
};
