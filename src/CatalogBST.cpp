#include "CatalogBST.h"

#include <algorithm>
#include <sstream>

#include "StringUtil.h"

namespace jc8 {

std::size_t CatalogBST::size() const {
    return size_;
}

bool CatalogBST::isEmpty() const {
    return size_ == 0;
}

bool CatalogBST::insert(const ProductPtr& product) {
    if (!product) {
        return false;
    }
    std::unique_ptr<Node>* link = &root_;
    while (*link) {
        int cmp = product->code().compare((*link)->product->code());
        if (cmp == 0) {
            return false;
        }
        link = cmp < 0 ? &(*link)->left : &(*link)->right;
    }
    *link = std::make_unique<Node>(product);
    ++size_;
    return true;
}

ProductPtr CatalogBST::search(const std::string& code) const {
    std::string key = toUpper(trim(code));
    const Node* current = root_.get();
    while (current) {
        int cmp = key.compare(current->product->code());
        if (cmp == 0) {
            return current->product;
        }
        current = cmp < 0 ? current->left.get() : current->right.get();
    }
    return nullptr;
}

ProductPtr CatalogBST::searchByName(const std::string& name) const {
    return findByName(root_.get(), toLower(trim(name)));
}

ProductPtr CatalogBST::findByName(const Node* node, const std::string& name) {
    if (!node) {
        return nullptr;
    }
    ProductPtr found = findByName(node->left.get(), name);
    if (found) {
        return found;
    }
    if (toLower(node->product->name()) == name || toLower(node->product->fullName()) == name) {
        return node->product;
    }
    return findByName(node->right.get(), name);
}

std::vector<ProductPtr> CatalogBST::searchByKeyword(const std::string& query) const {
    std::vector<std::string> tokens;
    std::istringstream stream(toLower(trim(query)));
    std::string token;
    while (stream >> token) {
        tokens.push_back(token);
    }
    std::vector<ProductPtr> out;
    collect(root_.get(), tokens, out);
    return out;
}

void CatalogBST::collect(const Node* node, const std::vector<std::string>& tokens,
                         std::vector<ProductPtr>& out) {
    if (!node) {
        return;
    }
    collect(node->left.get(), tokens, out);
    std::string haystack = toLower(node->product->code() + " " + node->product->fullName());
    bool match = true;
    for (const std::string& t : tokens) {
        if (haystack.find(t) == std::string::npos) {
            match = false;
            break;
        }
    }
    if (match) {
        out.push_back(node->product);
    }
    collect(node->right.get(), tokens, out);
}

std::vector<ProductPtr> CatalogBST::toList() const {
    std::vector<ProductPtr> out;
    out.reserve(size_);
    collect(root_.get(), {}, out);
    return out;
}

bool CatalogBST::remove(const std::string& code) {
    bool removed = removeFrom(root_, toUpper(trim(code)));
    if (removed) {
        --size_;
    }
    return removed;
}

bool CatalogBST::removeFrom(std::unique_ptr<Node>& node, const std::string& key) {
    if (!node) {
        return false;
    }
    int cmp = key.compare(node->product->code());
    if (cmp < 0) {
        return removeFrom(node->left, key);
    }
    if (cmp > 0) {
        return removeFrom(node->right, key);
    }
    if (!node->left) {
        node = std::move(node->right);
        return true;
    }
    if (!node->right) {
        node = std::move(node->left);
        return true;
    }
    const Node* successor = node->right.get();
    while (successor->left) {
        successor = successor->left.get();
    }
    node->product = successor->product;
    std::string successorCode = successor->product->code();
    return removeFrom(node->right, successorCode);
}

int CatalogBST::height() const {
    return heightOf(root_.get());
}

int CatalogBST::heightOf(const Node* node) {
    if (!node) {
        return 0;
    }
    return 1 + std::max(heightOf(node->left.get()), heightOf(node->right.get()));
}

void CatalogBST::rebuildBalanced(std::vector<ProductPtr> products) {
    std::sort(products.begin(), products.end(),
              [](const ProductPtr& a, const ProductPtr& b) { return a->code() < b->code(); });
    products.erase(std::unique(products.begin(), products.end(),
                               [](const ProductPtr& a, const ProductPtr& b) {
                                   return a->code() == b->code();
                               }),
                   products.end());
    root_ = build(products, 0, static_cast<int>(products.size()) - 1);
    size_ = products.size();
}

std::unique_ptr<CatalogBST::Node> CatalogBST::build(const std::vector<ProductPtr>& list, int lo,
                                                     int hi) {
    if (lo > hi) {
        return nullptr;
    }
    int mid = lo + (hi - lo) / 2;
    auto node = std::make_unique<Node>(list[mid]);
    node->left = build(list, lo, mid - 1);
    node->right = build(list, mid + 1, hi);
    return node;
}

}
