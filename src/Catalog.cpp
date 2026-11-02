#include "Catalog.h"

#include <algorithm>
#include <cstdio>
#include <stdexcept>

#include "CatalogLoader.h"
#include "StringUtil.h"

namespace jc8 {

Catalog Catalog::fromCsv(const std::string& path) {
    Catalog catalog;
    catalog.source_ = path;
    catalog.tree_.rebuildBalanced(CatalogLoader::load(path));
    return catalog;
}

std::size_t Catalog::size() const {
    return tree_.size();
}

int Catalog::treeHeight() const {
    return tree_.height();
}

ProductPtr Catalog::findByCode(const std::string& code) const {
    return tree_.search(code);
}

ProductPtr Catalog::findByName(const std::string& name) const {
    return tree_.searchByName(name);
}

std::vector<ProductPtr> Catalog::search(const std::string& query) const {
    std::string q = trim(query);
    if (q.empty()) {
        return getAllProducts();
    }
    ProductPtr exact = tree_.search(q);
    if (exact) {
        return {exact};
    }
    return tree_.searchByKeyword(q);
}

double Catalog::getPrice(const std::string& code, PackType type) const {
    ProductPtr p = tree_.search(code);
    if (!p) {
        throw std::invalid_argument("Unknown item code: " + code);
    }
    return p->getPrice(type);
}

bool Catalog::isAvailable(const std::string& code, PackType type) const {
    ProductPtr p = tree_.search(code);
    return p && p->hasPrice(type);
}

std::vector<ProductPtr> Catalog::getAllProducts() const {
    return tree_.toList();
}

std::string Catalog::nextCode() const {
    int maxNumber = 0;
    for (const ProductPtr& p : tree_.toList()) {
        const std::string& code = p->code();
        if (code.rfind("JC8-", 0) != 0 || code.size() == 4) {
            continue;
        }
        std::string digits = code.substr(4);
        if (digits.find_first_not_of("0123456789") != std::string::npos) {
            continue;
        }
        maxNumber = std::max(maxNumber, std::stoi(digits));
    }
    char buf[32];
    std::snprintf(buf, sizeof(buf), "JC8-%04d", maxNumber + 1);
    return buf;
}

bool Catalog::addProduct(const ProductPtr& product) {
    bool added = tree_.insert(product);
    if (added) {
        int bits = 0;
        for (std::size_t n = std::max<std::size_t>(1, tree_.size()); n > 0; n >>= 1) {
            ++bits;
        }
        if (tree_.height() > 2 * bits + 2) {
            tree_.rebuildBalanced(tree_.toList());
        }
    }
    return added;
}

bool Catalog::removeProduct(const std::string& code) {
    return tree_.remove(code);
}

bool Catalog::updatePrice(const std::string& code, PackType type, double price) {
    ProductPtr p = tree_.search(code);
    if (!p) {
        return false;
    }
    p->setPrice(type, price);
    return true;
}

void Catalog::save() const {
    if (!source_.empty()) {
        CatalogLoader::save(source_, tree_.toList());
    }
}

void Catalog::saveAs(const std::string& path) {
    source_ = path;
    save();
}

}
