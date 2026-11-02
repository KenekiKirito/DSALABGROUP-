#pragma once

#include <utility>

namespace jc8 {

template <typename T>
class Stack {
private:
    struct Node {
        T data;
        Node* next = nullptr;

        Node(const T& value, Node* nextNode) : data(value), next(nextNode) {
        }

        Node(T&& value, Node* nextNode) : data(std::move(value)), next(nextNode) {
        }
    };

public:
    Stack() = default;

    Stack(const Stack& other) {
        try {
            copyFrom(other);
        } catch (...) {
            clear();
            throw;
        }
    }

    Stack& operator=(const Stack& other) {
        if (this != &other) {
            Stack copy(other);
            swap(copy);
        }
        return *this;
    }

    Stack(Stack&& other) noexcept : top_(other.top_), size_(other.size_) {
        other.top_ = nullptr;
        other.size_ = 0;
    }

    Stack& operator=(Stack&& other) noexcept {
        if (this != &other) {
            clear();
            top_ = other.top_;
            size_ = other.size_;
            other.top_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    ~Stack() {
        clear();
    }

    void push(const T& value) {
        top_ = new Node(value, top_);
        ++size_;
    }

    void push(T&& value) {
        top_ = new Node(std::move(value), top_);
        ++size_;
    }

    bool tryPop(T& out) {
        if (top_ == nullptr) {
            return false;
        }

        Node* node = top_;
        out = std::move(node->data);
        top_ = node->next;
        delete node;
        --size_;
        return true;
    }

    void pop() {
        if (top_ == nullptr) {
            return;
        }
        Node* node = top_;
        top_ = node->next;
        delete node;
        --size_;
    }

    bool tryTop(T& out) const {
        if (top_ == nullptr) {
            return false;
        }
        out = top_->data;
        return true;
    }

    T& top() { return top_->data; }
    const T& top() const { return top_->data; }

    bool isEmpty() const { return top_ == nullptr; }
    int size() const { return size_; }

    void clear() {
        while (top_ != nullptr) {
            Node* next = top_->next;
            delete top_;
            top_ = next;
        }
        size_ = 0;
    }

    void dropBottom() {
        if (top_ == nullptr) {
            return;
        }
        if (top_->next == nullptr) {
            delete top_;
            top_ = nullptr;
            --size_;
            return;
        }

        Node* previous = top_;
        while (previous->next != nullptr && previous->next->next != nullptr) {
            previous = previous->next;
        }
        delete previous->next;
        previous->next = nullptr;
        --size_;
    }

private:
    Node* top_ = nullptr;
    int size_ = 0;

    void copyFrom(const Stack& other) {
        Stack temporary;
        for (const Node* current = other.top_; current != nullptr;
             current = current->next) {
            temporary.push(current->data);
        }
        while (!temporary.isEmpty()) {
            T value = std::move(temporary.top());
            temporary.pop();
            push(std::move(value));
        }
    }

    void swap(Stack& other) noexcept {
        std::swap(top_, other.top_);
        std::swap(size_, other.size_);
    }
};

} // namespace jc8
