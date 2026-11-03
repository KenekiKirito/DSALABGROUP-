#include "SalesAnalysis.h"

#include <map>

namespace jc8 {

std::vector<ProductStat> aggregateSales(const Catalog& catalog, const std::vector<Invoice>& log) {
    std::vector<ProductStat> stats;
    std::map<std::string, std::size_t> indexByCode;
    for (const ProductPtr& p : catalog.getAllProducts()) {
        ProductStat stat;
        stat.code = p->code();
        stat.name = p->fullName();
        indexByCode[stat.code] = stats.size();
        stats.push_back(stat);
    }
    for (const Invoice& invoice : log) {
        for (const LineItem& item : invoice.items) {
            auto it = indexByCode.find(item.code);
            if (it == indexByCode.end()) {
                ProductStat stat;
                stat.code = item.code;
                stat.name = item.name;
                it = indexByCode.emplace(item.code, stats.size()).first;
                stats.push_back(stat);
            }
            stats[it->second].unitsSold += item.quantity;
            stats[it->second].revenue += item.lineTotal();
        }
    }
    return stats;
}

MinMaxResult findMinMax(const std::vector<ProductStat>& stats) {
    MinMaxResult r;
    for (std::size_t i = 0; i < stats.size(); ++i) {
        int index = static_cast<int>(i);
        if (stats[i].unitsSold == 0) {
            r.slowMoving.push_back(index);
            continue;
        }
        if (r.maxUnits < 0 || stats[i].unitsSold > stats[r.maxUnits].unitsSold) {
            r.maxUnits = index;
        }
        if (r.minUnits < 0 || stats[i].unitsSold < stats[r.minUnits].unitsSold) {
            r.minUnits = index;
        }
        if (r.maxRevenue < 0 || stats[i].revenue > stats[r.maxRevenue].revenue) {
            r.maxRevenue = index;
        }
        if (r.minRevenue < 0 || stats[i].revenue < stats[r.minRevenue].revenue) {
            r.minRevenue = index;
        }
    }
    return r;
}

}
