#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "PackType.h"

namespace jc8 {

class Product {
public:
    Product(const std::string& code, const std::string& name, const std::string& category,
            PackType defaultPack, double basePrice);

    const std::string& code() const;
    const std::string& name() const;
    const std::string& category() const;
    std::string fullName() const;
    PackType defaultPack() const;
    double basePrice() const;

    void setName(const std::string& name);
    void setCategory(const std::string& category);

    bool hasPrice(PackType type) const;
    double getPrice(PackType type) const;
    void setPrice(PackType type, double price);
    bool removePrice(PackType type);
    std::vector<PackType> availablePacks() const;
    const std::map<PackType, double>& prices() const;

    std::string toString() const;

private:
    std::string code_;
    std::string name_;
    std::string category_;
    PackType defaultPack_;
    std::map<PackType, double> prices_;
};

using ProductPtr = std::shared_ptr<Product>;

}
