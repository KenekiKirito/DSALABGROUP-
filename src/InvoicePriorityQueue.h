#pragma once

#include <cstddef>
#include <vector>

#include "Invoice.h"

namespace jc8 {

// Priority queue (max-heap) of invoices waiting to be logged.
// Higher total goes first. If totals are equal, the earlier invoice goes first.
class InvoicePriorityQueue {
public:
    std::size_t size() const;
    bool isEmpty() const;

    void enqueue(const Invoice& invoice);
    Invoice dequeue();
    const Invoice& peek() const;

    // Lists the pending invoices in priority order without removing them.
    std::vector<Invoice> toPriorityList() const;

private:
    static bool higherPriority(const Invoice& a, const Invoice& b);
    void siftUp(std::size_t index);
    void siftDown(std::size_t index);

    std::vector<Invoice> heap_;
};

}
