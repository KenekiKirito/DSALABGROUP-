#pragma once

#include <vector>

#include "Invoice.h"

namespace jc8 {

// Returns true if a should come before b.
using InvoiceOrder = bool (*)(const Invoice& a, const Invoice& b);

bool byDate(const Invoice& a, const Invoice& b);
bool byTotalDesc(const Invoice& a, const Invoice& b);
bool byItemCountDesc(const Invoice& a, const Invoice& b);

// Quick sort. Average time is O(n log n).
void quickSort(std::vector<Invoice>& invoices, InvoiceOrder before);

}
