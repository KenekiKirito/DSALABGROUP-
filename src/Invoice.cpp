#include "Invoice.h"

#include <cmath>
#include <cstdio>

namespace jc8 {

Money toCentavos(double pesos) {
    return static_cast<Money>(std::llround(pesos * 100.0));
}

std::string formatMoney(Money centavos) {
    char buf[48];
    const char* sign = centavos < 0 ? "-" : "";
    Money magnitude = centavos < 0 ? -centavos : centavos;
    std::snprintf(buf, sizeof(buf), "%s%lld.%02lld", sign, magnitude / 100, magnitude % 100);
    return buf;
}

std::string formatTime(std::time_t time) {
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", std::localtime(&time));
    return buf;
}

Money LineItem::lineTotal() const {
    return unitPrice * quantity;
}

void Invoice::recompute() {
    subtotal = 0;
    itemCount = 0;
    for (const LineItem& item : items) {
        subtotal += item.lineTotal();
        itemCount += item.quantity;
    }
    tax = (subtotal * kTaxPercent + 50) / 100;
    total = subtotal + tax;
}

}
