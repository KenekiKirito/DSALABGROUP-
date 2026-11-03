#include "SalesLogStore.h"

#include <fstream>
#include <map>
#include <stdexcept>

#include "CatalogLoader.h"
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

}

std::vector<Invoice> SalesLogStore::load(const std::string& path) {
    std::vector<Invoice> log;
    std::ifstream in(path);
    if (!in) {
        return log;
    }
    std::map<int, std::size_t> indexById;
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
        std::vector<std::string> f = CatalogLoader::parseLine(line);
        if (f.size() < 8) {
            continue;
        }
        try {
            int id = std::stoi(trim(f[0]));
            auto it = indexById.find(id);
            if (it == indexById.end()) {
                Invoice invoice;
                invoice.id = id;
                invoice.finalizedAt = static_cast<std::time_t>(std::stoll(trim(f[1])));
                invoice.paid = std::stoll(trim(f[2]));
                it = indexById.emplace(id, log.size()).first;
                log.push_back(invoice);
            }
            LineItem item;
            item.code = toUpper(trim(f[3]));
            item.name = f[4];
            item.pack = parsePackType(f[5]);
            item.quantity = std::stoi(trim(f[6]));
            item.unitPrice = std::stoll(trim(f[7]));
            log[it->second].items.push_back(item);
        } catch (const std::exception&) {
            continue;
        }
    }
    for (Invoice& invoice : log) {
        invoice.recompute();
        invoice.change = invoice.paid - invoice.total;
    }
    return log;
}

void SalesLogStore::save(const std::string& path, const std::vector<Invoice>& log) {
    std::ofstream out(path, std::ios::trunc);
    if (!out) {
        throw std::runtime_error("Cannot write sales log: " + path);
    }
    out << "invoice,timestamp,paid,code,name,pack,quantity,unitPrice\n";
    for (const Invoice& invoice : log) {
        for (const LineItem& item : invoice.items) {
            out << invoice.id << ',' << static_cast<long long>(invoice.finalizedAt) << ','
                << invoice.paid << ',' << escapeField(item.code) << ',' << escapeField(item.name)
                << ',' << packName(item.pack) << ',' << item.quantity << ',' << item.unitPrice
                << '\n';
        }
    }
}

}
