#pragma once

#include <cmath>
#include <string>

namespace jc8 {

enum class PackageType {
    Set = 0,
    Rolls = 1,
    Box = 2
};

inline constexpr int PackageTypeCount = 3;

const char* packageTypeName(PackageType type);
bool packageTypeFromString(const std::string& text, PackageType& out);
bool isValidPackageType(PackageType type);
int packageTypeIndex(PackageType type);

inline double roundMoney(double value) {
    if (!std::isfinite(value)) {
        return value;
    }
    const double rounded = std::round(value * 100.0) / 100.0;
    return rounded == 0.0 ? 0.0 : rounded;
}

struct Product {
    std::string code;
    std::string name;
    double prices[PackageTypeCount] = {0.0, 0.0, 0.0};

    Product() = default;
    Product(std::string productCode,
            std::string productName,
            double setPrice,
            double rollPrice,
            double boxPrice);

    double priceFor(PackageType type) const;
};

struct InvoiceItem {
    int lineId = 0;
    std::string productCode;
    std::string description;
    PackageType package = PackageType::Set;
    double unitPrice = 0.0;
    int quantity = 0;

    double lineTotal() const;
};

} // namespace jc8
