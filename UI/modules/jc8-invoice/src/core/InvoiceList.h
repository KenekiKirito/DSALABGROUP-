#pragma once

#include "Types.h"

namespace jc8 {

// Active invoice storage: a hand-built singly linked list.
// No STL container is used for the list or its nodes.
class InvoiceList {
private:
    struct Node {
        InvoiceItem item;
        Node* next = nullptr;

        explicit Node(const InvoiceItem& value) : item(value) {
        }
    };

public:
    class Iterator {
    public:
        Iterator() = default;

        InvoiceItem& operator*() const { return node_->item; }
        InvoiceItem* operator->() const { return &node_->item; }

        Iterator& operator++() {
            if (node_ != nullptr) {
                node_ = node_->next;
            }
            return *this;
        }

        Iterator operator++(int) {
            Iterator copy = *this;
            ++(*this);
            return copy;
        }

        bool operator==(const Iterator& other) const { return node_ == other.node_; }
        bool operator!=(const Iterator& other) const { return node_ != other.node_; }

    private:
        explicit Iterator(Node* node) : node_(node) {
        }

        Node* node_ = nullptr;
        friend class InvoiceList;
    };

    class ConstIterator {
    public:
        ConstIterator() = default;

        const InvoiceItem& operator*() const { return node_->item; }
        const InvoiceItem* operator->() const { return &node_->item; }

        ConstIterator& operator++() {
            if (node_ != nullptr) {
                node_ = node_->next;
            }
            return *this;
        }

        ConstIterator operator++(int) {
            ConstIterator copy = *this;
            ++(*this);
            return copy;
        }

        bool operator==(const ConstIterator& other) const { return node_ == other.node_; }
        bool operator!=(const ConstIterator& other) const { return node_ != other.node_; }

    private:
        explicit ConstIterator(const Node* node) : node_(node) {
        }

        const Node* node_ = nullptr;
        friend class InvoiceList;
    };

    InvoiceList() = default;
    InvoiceList(const InvoiceList& other);
    InvoiceList& operator=(const InvoiceList& other);
    InvoiceList(InvoiceList&& other) noexcept;
    InvoiceList& operator=(InvoiceList&& other) noexcept;
    ~InvoiceList();

    bool isEmpty() const { return head_ == nullptr; }
    int size() const { return size_; }

    void append(const InvoiceItem& item);
    bool removeById(int lineId);
    void clear();

    InvoiceItem* findById(int lineId);
    const InvoiceItem* findById(int lineId) const;

    // Position-based lookup is convenient for QAbstractTableModel::data().
    InvoiceItem* itemAt(int index);
    const InvoiceItem* itemAt(int index) const;
    int indexOf(int lineId) const;

    Iterator begin() { return Iterator(head_); }
    Iterator end() { return Iterator(); }
    ConstIterator begin() const { return ConstIterator(head_); }
    ConstIterator end() const { return ConstIterator(); }
    ConstIterator cbegin() const { return ConstIterator(head_); }
    ConstIterator cend() const { return ConstIterator(); }

private:
    Node* head_ = nullptr;
    Node* tail_ = nullptr;
    int size_ = 0;

    void copyFrom(const InvoiceList& other);
    void swap(InvoiceList& other) noexcept;
};

} // namespace jc8
