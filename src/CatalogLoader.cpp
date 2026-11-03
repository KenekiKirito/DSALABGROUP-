#include "CatalogLoader.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "StringUtil.h"

namespace jc8 {

namespace {

std::string escapeField(const std::string& value) {
    if (value.find_first_of(",\"\n") == std::string::npos) {
        return value;
    }
    std::string out = "\"";
    for (char c : value) {
        if (c == '"') {
            out += '"';
        }
        out += c;
    }
    out += '"';
    return out;
}

std::string money(double value) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.2f", value);
    return buf;
}

void applyExtraPrices(Product& product, const std::string& text) {
    std::stringstream stream(text);
    std::string part;
    while (std::getline(stream, part, ';')) {
        std::size_t eq = part.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        product.setPrice(parsePackType(part.substr(0, eq)), std::stod(trim(part.substr(eq + 1))));
    }
}

}

std::vector<ProductPtr> CatalogLoader::load(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Cannot open catalog file: " + path);
    }
    std::vector<ProductPtr> products;
    std::string line;
    bool header = true;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (trim(line).empty()) {
            continue;
        }
        if (header) {
            header = false;
            continue;
        }
        std::vector<std::string> f = parseLine(line);
        if (f.size() < 5) {
            continue;
        }
        try {
            auto product = std::make_shared<Product>(f[0], f[1], f[2], parsePackType(f[3]),
                                                     std::stod(trim(f[4])));
            if (f.size() > 5) {
                applyExtraPrices(*product, f[5]);
            }
            products.push_back(product);
        } catch (const std::exception&) {
            continue;
        }
    }
    return products;
}

void CatalogLoader::save(const std::string& path, const std::vector<ProductPtr>& products) {
    std::ofstream out(path, std::ios::trunc);
    if (!out) {
        throw std::runtime_error("Cannot write catalog file: " + path);
    }
    out << "code,name,category,unit,price,extraPrices\n";
    for (const ProductPtr& p : products) {
        std::string extra;
        for (const auto& entry : p->prices()) {
            if (entry.first == p->defaultPack()) {
                continue;
            }
            if (!extra.empty()) {
                extra += ';';
            }
            extra += packName(entry.first) + "=" + money(entry.second);
        }
        out << escapeField(p->code()) << ',' << escapeField(p->name()) << ','
            << escapeField(p->category()) << ',' << packName(p->defaultPack()) << ','
            << money(p->basePrice()) << ',' << escapeField(extra) << '\n';
    }
}

std::vector<std::string> CatalogLoader::parseLine(const std::string& line) {
    std::vector<std::string> fields;
    std::string current;
    bool quoted = false;
    for (std::size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (quoted) {
            if (c == '"') {
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    current += '"';
                    ++i;
                } else {
                    quoted = false;
                }
            } else {
                current += c;
            }
        } else if (c == '"') {
            quoted = true;
        } else if (c == ',') {
            fields.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }
    fields.push_back(current);
    return fields;
}

}
