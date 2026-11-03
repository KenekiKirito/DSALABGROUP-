#pragma once

#include <string>
#include <vector>

#include "Catalog.h"
#include "Invoice.h"

namespace jc8 {

struct ProductStat {
    std::string code;
    std::string name;
    int unitsSold = 0;
    Money revenue = 0;
};

// Positions in the stats list. -1 means no product has sales.
struct MinMaxResult {
    int maxUnits = -1;
    int minUnits = -1;
    int maxRevenue = -1;
    int minRevenue = -1;
    std::vector<int> slowMoving;
};

// Adds up units sold and revenue for each product in the catalog.
std::vector<ProductStat> aggregateSales(const Catalog& catalog, const std::vector<Invoice>& log);

// Finds the max and min in one pass. Products with no sales are slow-moving.
MinMaxResult findMinMax(const std::vector<ProductStat>& stats);

}
