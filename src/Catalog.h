#pragma once

#include <string>
#include <vector>

#include "CatalogBST.h"
#include "PackType.h"
#include "Product.h"

namespace jc8 {

class Catalog {
public:
    Catalog() = default;

    static Catalog fromCsv(const std::string& path);

    std::size_t size() const;
    int treeHeight() const;

    ProductPtr findByCode(const std::string& code) const;
    ProductPtr findByName(const std::string& name) const;
    std::vector<ProductPtr> search(const std::string& query) const;

    double getPrice(const std::string& code, PackType type) const;
    bool isAvailable(const std::string& code, PackType type) const;
    std::vector<ProductPtr> getAllProducts() const;

    std::string nextCode() const;
    bool addProduct(const ProductPtr& product);
    bool removeProduct(const std::string& code);
    bool updatePrice(const std::string& code, PackType type, double price);

    void save() const;
    void saveAs(const std::string& path);

private:
    CatalogBST tree_;
    std::string source_;
};

}
