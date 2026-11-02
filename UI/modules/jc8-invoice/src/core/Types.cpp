#include "Types.h"

#include <cctype>
#include <utility>

namespace jc8 {

const char* packageTypeName(PackageType type) {
    switch (type) {
    case PackageType::Set:
        return "Set";
    case PackageType::Rolls:
        return "Rolls";
    case PackageType::Box:
        return "Box";
    }
    return "Unknown";
}

bool packageTypeFromString(const std::string& text, PackageType& out) {
    std::string lowered;
    lowered.reserve(text.size());
    for (char character : text) {
        lowered.push_back(static_cast<char>(
            std::tolower(static_cast<unsigned char>(character))));
    }

    if (lowered == "set" || lowered == "sets") {
        out = PackageType::Set;
        return true;
    }
    if (lowered == "roll" || lowered == "rolls") {
        out = PackageType::Rolls;
        return true;
    }
    if (lowered == "box" || lowered == "boxes") {
        out = PackageType::Box;
        return true;
    }
    return false;
}

bool isValidPackageType(PackageType type) {
    const int index = static_cast<int>(type);
    return index >= 0 && index < PackageTypeCount;
}

int packageTypeIndex(PackageType type) {
    return isValidPackageType(type) ? static_cast<int>(type) : -1;
}

Product::Product(std::string productCode,
                 std::string productName,
                 double setPrice,
                 double rollPrice,
                 double boxPrice)
    : code(std::move(productCode)),
      name(std::move(productName)),
      prices{setPrice, rollPrice, boxPrice} {
}

double Product::priceFor(PackageType type) const {
    const int index = packageTypeIndex(type);
    return index >= 0 ? prices[index] : 0.0;
}

double InvoiceItem::lineTotal() const {
    return roundMoney(unitPrice * static_cast<double>(quantity));
}

} // namespace jc8
