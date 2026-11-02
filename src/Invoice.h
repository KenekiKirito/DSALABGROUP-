#pragma once

#include <ctime>
#include <string>
#include <vector>

#include "PackType.h"

namespace jc8 {

// Money is stored in centavos (whole numbers) to avoid decimal errors.
using Money = long long;

constexpr int kTaxPercent = 12;

Money toCentavos(double pesos);
std::string formatMoney(Money centavos);
std::string formatTime(std::time_t time);

struct LineItem {
    std::string code;
    std::string name;
    PackType pack = PackType::Piece;
    int quantity = 0;
    Money unitPrice = 0;

    Money lineTotal() const;
};

struct Invoice {
    int id = 0;
    std::time_t finalizedAt = 0;
    std::vector<LineItem> items;
    Money subtotal = 0;
    Money tax = 0;
    Money total = 0;
    int itemCount = 0;
    Money paid = 0;
    Money change = 0;

    // Recalculates subtotal, tax, total and item count.
    void recompute();
};

}
