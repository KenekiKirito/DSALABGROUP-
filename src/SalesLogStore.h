#pragma once

#include <string>
#include <vector>

#include "Invoice.h"

namespace jc8 {

// Saves and loads the sales log as a CSV file (one row per item).
class SalesLogStore {
public:
    static std::vector<Invoice> load(const std::string& path);
    static void save(const std::string& path, const std::vector<Invoice>& log);
};

}
