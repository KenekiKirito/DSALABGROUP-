#include "Product.h"

#include <cstdio>
#include <stdexcept>

#include "StringUtil.h"

namespace jc8 {

Product::Product(const std::string& code, const std::string& name, const std::string& category,
                 PackType defaultPack, double basePrice)
    : code_(toUpper(trim(code))),
      name_(trim(name)),
      category_(trim(category)),
      defaultPack_(defaultPack) {
    if (code_.empty()) {
        throw std::invalid_argument("Item code is required");
    }
    if (basePrice < 0) {
        throw std::invalid_argument("Price cannot be negative");
    }
    prices_[defaultPack_] = basePrice;
}

const std::string& Product::code() const {
    return code_;
}

const std::string& Product::name() const {
    return name_;
}

const std::string& Product::category() const {
    return category_;
}

std::string Product::fullName() const {
    if (category_.empty() || toLower(name_).rfind(toLower(category_), 0) == 0) {
        return name_;
    }
    return category_ + " - " + name_;
}

PackType Product::defaultPack() const {
    return defaultPack_;
}

double Product::basePrice() const {
    return prices_.at(defaultPack_);
}

void Product::setName(const std::string& name) {
    name_ = trim(name);
}

void Product::setCategory(const std::string& category) {
    category_ = trim(category);
}

bool Product::hasPrice(PackType type) const {
    return prices_.count(type) > 0;
}

double Product::getPrice(PackType type) const {
    auto it = prices_.find(type);
    if (it == prices_.end()) {
        throw std::invalid_argument(code_ + " has no " + packLabel(type) + " price");
    }
    return it->second;
}

void Product::setPrice(PackType type, double price) {
    if (price < 0) {
        throw std::invalid_argument("Price cannot be negative");
    }
    prices_[type] = price;
}

bool Product::removePrice(PackType type) {
    if (type == defaultPack_) {
        return false;
    }
    return prices_.erase(type) > 0;
}

std::vector<PackType> Product::availablePacks() const {
    std::vector<PackType> out;
    for (const auto& entry : prices_) {
        out.push_back(entry.first);
    }
    return out;
}

const std::map<PackType, double>& Product::prices() const {
    return prices_;
}

std::string Product::toString() const {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.2f", basePrice());
    return code_ + " | " + fullName() + " | " + packLabel(defaultPack_) + " | " + buf;
}

}
