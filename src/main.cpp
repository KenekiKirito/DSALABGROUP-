#include <cstdio>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "Catalog.h"
#include "StringUtil.h"

using namespace jc8;

namespace {

std::string prompt(const std::string& label) {
    std::cout << label;
    std::string line;
    std::getline(std::cin, line);
    return trim(line);
}

void printProducts(const std::vector<ProductPtr>& list) {
    for (const ProductPtr& p : list) {
        std::printf("%-9s %-62.62s %-7s %10.2f\n", p->code().c_str(), p->fullName().c_str(),
                    packLabel(p->defaultPack()).c_str(), p->basePrice());
    }
    std::printf("%zu item(s)\n", list.size());
}

PackType askPackType(PackType fallback) {
    const std::vector<PackType>& all = allPackTypes();
    for (std::size_t i = 0; i < all.size(); ++i) {
        std::printf("  %zu) %s\n", i + 1, packLabel(all[i]).c_str());
    }
    std::string choice = prompt("Type number: ");
    try {
        std::size_t index = static_cast<std::size_t>(std::stoul(choice));
        if (index >= 1 && index <= all.size()) {
            return all[index - 1];
        }
    } catch (const std::exception&) {
    }
    return fallback;
}

void searchItems(const Catalog& catalog) {
    printProducts(catalog.search(prompt("Search code or name (blank = all): ")));
}

void lookupByCode(const Catalog& catalog) {
    ProductPtr p = catalog.findByCode(prompt("Item code: "));
    if (!p) {
        std::cout << "Not found\n";
        return;
    }
    std::cout << p->toString() << "\n";
    for (const auto& entry : p->prices()) {
        std::printf("  %-8s %10.2f\n", packLabel(entry.first).c_str(), entry.second);
    }
}

void lookupByName(const Catalog& catalog) {
    ProductPtr p = catalog.findByName(prompt("Exact item name: "));
    std::cout << (p ? p->toString() : std::string("Not found")) << "\n";
}

void addItem(Catalog& catalog) {
    std::string name = prompt("Name: ");
    if (name.empty()) {
        std::cout << "Name is required\n";
        return;
    }
    std::string category = prompt("Category: ");
    PackType type = askPackType(PackType::Piece);
    try {
        double price = std::stod(prompt("Price: "));
        auto product = std::make_shared<Product>(catalog.nextCode(), name, category, type, price);
        catalog.addProduct(product);
        catalog.save();
        std::cout << "Added " << product->toString() << "\n";
    } catch (const std::exception& e) {
        std::cout << "Could not add item: " << e.what() << "\n";
    }
}

void setPrice(Catalog& catalog) {
    ProductPtr p = catalog.findByCode(prompt("Item code: "));
    if (!p) {
        std::cout << "Not found\n";
        return;
    }
    PackType type = askPackType(p->defaultPack());
    try {
        double price = std::stod(prompt("New price: "));
        catalog.updatePrice(p->code(), type, price);
        catalog.save();
        std::cout << "Updated " << p->toString() << "\n";
    } catch (const std::exception& e) {
        std::cout << "Could not update price: " << e.what() << "\n";
    }
}

void removeItem(Catalog& catalog) {
    std::string code = prompt("Item code: ");
    if (catalog.removeProduct(code)) {
        catalog.save();
        std::cout << "Removed\n";
    } else {
        std::cout << "Not found\n";
    }
}

}

int main(int argc, char** argv) {
    std::string path = argc > 1 ? argv[1] : "data/products.csv";
    try {
        Catalog catalog = Catalog::fromCsv(path);
        std::printf("Loaded %zu items (tree height %d)\n", catalog.size(), catalog.treeHeight());
        while (true) {
            std::cout << "\n1) Search  2) Lookup by code  3) Lookup by name  4) Add item\n"
                         "5) Set price  6) Remove item  0) Exit\n";
            std::string choice = prompt("> ");
            if (choice == "0" || !std::cin) {
                break;
            }
            if (choice == "1") {
                searchItems(catalog);
            } else if (choice == "2") {
                lookupByCode(catalog);
            } else if (choice == "3") {
                lookupByName(catalog);
            } else if (choice == "4") {
                addItem(catalog);
            } else if (choice == "5") {
                setPrice(catalog);
            } else if (choice == "6") {
                removeItem(catalog);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
    return 0;
}
