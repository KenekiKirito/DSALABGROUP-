#include "SalesSort.h"

#include <utility>

namespace jc8 {

namespace {

int partition(std::vector<Invoice>& list, int low, int high, InvoiceOrder before) {
    int mid = low + (high - low) / 2;
    std::swap(list[mid], list[high]);
    const Invoice pivot = list[high];
    int boundary = low - 1;
    for (int i = low; i < high; ++i) {
        if (before(list[i], pivot)) {
            ++boundary;
            std::swap(list[boundary], list[i]);
        }
    }
    std::swap(list[boundary + 1], list[high]);
    return boundary + 1;
}

void sortRange(std::vector<Invoice>& list, int low, int high, InvoiceOrder before) {
    if (low >= high) {
        return;
    }
    int p = partition(list, low, high, before);
    sortRange(list, low, p - 1, before);
    sortRange(list, p + 1, high, before);
}

}

bool byDate(const Invoice& a, const Invoice& b) {
    if (a.finalizedAt != b.finalizedAt) {
        return a.finalizedAt < b.finalizedAt;
    }
    return a.id < b.id;
}

bool byTotalDesc(const Invoice& a, const Invoice& b) {
    if (a.total != b.total) {
        return a.total > b.total;
    }
    return a.id < b.id;
}

bool byItemCountDesc(const Invoice& a, const Invoice& b) {
    if (a.itemCount != b.itemCount) {
        return a.itemCount > b.itemCount;
    }
    return a.id < b.id;
}

void quickSort(std::vector<Invoice>& invoices, InvoiceOrder before) {
    sortRange(invoices, 0, static_cast<int>(invoices.size()) - 1, before);
}

}
