#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Product.h"

namespace jc8 {

class CatalogBST {
public:
    std::size_t size() const;
    bool isEmpty() const;

    bool insert(const ProductPtr& product);
    ProductPtr search(const std::string& code) const;
    ProductPtr searchByName(const std::string& name) const;
    std::vector<ProductPtr> searchByKeyword(const std::string& query) const;
    std::vector<ProductPtr> toList() const;
    bool remove(const std::string& code);
    int height() const;
    void rebuildBalanced(std::vector<ProductPtr> products);

private:
    struct Node {
        explicit Node(const ProductPtr& p) : product(p) {}
        ProductPtr product;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
    };

    static ProductPtr findByName(const Node* node, const std::string& name);
    static void collect(const Node* node, const std::vector<std::string>& tokens,
                        std::vector<ProductPtr>& out);
    static bool removeFrom(std::unique_ptr<Node>& node, const std::string& key);
    static int heightOf(const Node* node);
    static std::unique_ptr<Node> build(const std::vector<ProductPtr>& list, int lo, int hi);

    std::unique_ptr<Node> root_;
    std::size_t size_ = 0;
};

}
