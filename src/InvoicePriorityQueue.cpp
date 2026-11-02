#include "InvoicePriorityQueue.h"

#include <stdexcept>
#include <utility>

namespace jc8 {

std::size_t InvoicePriorityQueue::size() const {
    return heap_.size();
}

bool InvoicePriorityQueue::isEmpty() const {
    return heap_.empty();
}

bool InvoicePriorityQueue::higherPriority(const Invoice& a, const Invoice& b) {
    if (a.total != b.total) {
        return a.total > b.total;
    }
    return a.id < b.id;
}

void InvoicePriorityQueue::enqueue(const Invoice& invoice) {
    heap_.push_back(invoice);
    siftUp(heap_.size() - 1);
}

Invoice InvoicePriorityQueue::dequeue() {
    if (heap_.empty()) {
        throw std::out_of_range("Priority queue is empty");
    }
    Invoice top = heap_.front();
    heap_.front() = heap_.back();
    heap_.pop_back();
    if (!heap_.empty()) {
        siftDown(0);
    }
    return top;
}

const Invoice& InvoicePriorityQueue::peek() const {
    if (heap_.empty()) {
        throw std::out_of_range("Priority queue is empty");
    }
    return heap_.front();
}

std::vector<Invoice> InvoicePriorityQueue::toPriorityList() const {
    InvoicePriorityQueue copy = *this;
    std::vector<Invoice> out;
    out.reserve(heap_.size());
    while (!copy.isEmpty()) {
        out.push_back(copy.dequeue());
    }
    return out;
}

void InvoicePriorityQueue::siftUp(std::size_t index) {
    while (index > 0) {
        std::size_t parent = (index - 1) / 2;
        if (!higherPriority(heap_[index], heap_[parent])) {
            break;
        }
        std::swap(heap_[index], heap_[parent]);
        index = parent;
    }
}

void InvoicePriorityQueue::siftDown(std::size_t index) {
    std::size_t n = heap_.size();
    while (true) {
        std::size_t left = 2 * index + 1;
        std::size_t right = left + 1;
        std::size_t best = index;
        if (left < n && higherPriority(heap_[left], heap_[best])) {
            best = left;
        }
        if (right < n && higherPriority(heap_[right], heap_[best])) {
            best = right;
        }
        if (best == index) {
            break;
        }
        std::swap(heap_[index], heap_[best]);
        index = best;
    }
}

}
