#include "InvoiceList.h"

#include <utility>

namespace jc8 {

InvoiceList::InvoiceList(const InvoiceList& other) {
    try {
        copyFrom(other);
    } catch (...) {
        clear();
        throw;
    }
}

InvoiceList& InvoiceList::operator=(const InvoiceList& other) {
    if (this != &other) {
        InvoiceList copy(other);
        swap(copy);
    }
    return *this;
}

InvoiceList::InvoiceList(InvoiceList&& other) noexcept
    : head_(other.head_), tail_(other.tail_), size_(other.size_) {
    other.head_ = nullptr;
    other.tail_ = nullptr;
    other.size_ = 0;
}

InvoiceList& InvoiceList::operator=(InvoiceList&& other) noexcept {
    if (this != &other) {
        clear();
        head_ = other.head_;
        tail_ = other.tail_;
        size_ = other.size_;
        other.head_ = nullptr;
        other.tail_ = nullptr;
        other.size_ = 0;
    }
    return *this;
}

InvoiceList::~InvoiceList() {
    clear();
}

void InvoiceList::append(const InvoiceItem& item) {
    Node* node = new Node(item);
    if (tail_ == nullptr) {
        head_ = node;
        tail_ = node;
    } else {
        tail_->next = node;
        tail_ = node;
    }
    ++size_;
}

bool InvoiceList::removeById(int lineId) {
    Node* previous = nullptr;
    Node* current = head_;

    while (current != nullptr) {
        if (current->item.lineId == lineId) {
            if (previous == nullptr) {
                head_ = current->next;
            } else {
                previous->next = current->next;
            }
            if (current == tail_) {
                tail_ = previous;
            }
            delete current;
            --size_;
            return true;
        }
        previous = current;
        current = current->next;
    }
    return false;
}

void InvoiceList::clear() {
    Node* current = head_;
    while (current != nullptr) {
        Node* next = current->next;
        delete current;
        current = next;
    }
    head_ = nullptr;
    tail_ = nullptr;
    size_ = 0;
}

InvoiceItem* InvoiceList::findById(int lineId) {
    for (Node* current = head_; current != nullptr; current = current->next) {
        if (current->item.lineId == lineId) {
            return &current->item;
        }
    }
    return nullptr;
}

const InvoiceItem* InvoiceList::findById(int lineId) const {
    for (const Node* current = head_; current != nullptr; current = current->next) {
        if (current->item.lineId == lineId) {
            return &current->item;
        }
    }
    return nullptr;
}

InvoiceItem* InvoiceList::itemAt(int index) {
    if (index < 0) {
        return nullptr;
    }

    Node* current = head_;
    for (int position = 0; current != nullptr; ++position, current = current->next) {
        if (position == index) {
            return &current->item;
        }
    }
    return nullptr;
}

const InvoiceItem* InvoiceList::itemAt(int index) const {
    if (index < 0) {
        return nullptr;
    }

    const Node* current = head_;
    for (int position = 0; current != nullptr; ++position, current = current->next) {
        if (position == index) {
            return &current->item;
        }
    }
    return nullptr;
}

int InvoiceList::indexOf(int lineId) const {
    int position = 0;
    for (const Node* current = head_; current != nullptr;
         current = current->next, ++position) {
        if (current->item.lineId == lineId) {
            return position;
        }
    }
    return -1;
}

void InvoiceList::copyFrom(const InvoiceList& other) {
    for (const InvoiceItem& item : other) {
        append(item);
    }
}

void InvoiceList::swap(InvoiceList& other) noexcept {
    std::swap(head_, other.head_);
    std::swap(tail_, other.tail_);
    std::swap(size_, other.size_);
}

} // namespace jc8
