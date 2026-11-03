#pragma once

#include <string>
#include <vector>

#include "Product.h"

namespace jc8 {

class CatalogLoader {
public:
    static std::vector<ProductPtr> load(const std::string& path);
    static void save(const std::string& path, const std::vector<ProductPtr>& products);
    static std::vector<std::string> parseLine(const std::string& line);
};

}
