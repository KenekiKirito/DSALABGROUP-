#include <cstdio>
#include <ctime>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Catalog.h"
#include "Invoice.h"
#include "InvoicePriorityQueue.h"
#include "SalesAnalysis.h"
#include "SalesLogStore.h"
#include "SalesSort.h"
#include "StringUtil.h"

using namespace jc8;

namespace {

const std::size_t kMaxChoices = 15;
const std::size_t kMaxSlowListed = 10;

std::string prompt(const std::string& label) {
    std::cout << label;
    std::string line;
    if (!std::getline(std::cin, line)) {
        throw std::runtime_error("Input closed");
    }
    return trim(line);
}

int readInt(const std::string& label, int lo, int hi) {
    while (true) {
        std::string text = prompt(label);
        try {
            std::size_t used = 0;
            int value = std::stoi(text, &used);
            if (used == text.size() && value >= lo && value <= hi) {
                return value;
            }
        } catch (const std::exception&) {
        }
        std::printf("Enter a number from %d to %d\n", lo, hi);
    }
}

Money readPesos(const std::string& label) {
    while (true) {
        std::string text = prompt(label);
        try {
            std::size_t used = 0;
            double value = std::stod(text, &used);
            if (used == text.size() && value >= 0) {
                return toCentavos(value);
            }
        } catch (const std::exception&) {
        }
        std::printf("Enter a valid amount\n");
    }
}

void printInvoice(const Invoice& invoice) {
    std::printf("\nInvoice #%d  %s\n", invoice.id, formatTime(invoice.finalizedAt).c_str());
    for (const LineItem& item : invoice.items) {
        std::printf("  %-9s %-40.40s %-7s x%-4d %12s\n", item.code.c_str(), item.name.c_str(),
                    packLabel(item.pack).c_str(), item.quantity,
                    formatMoney(item.lineTotal()).c_str());
    }
    std::printf("  Subtotal %s | VAT %d%% %s | TOTAL %s\n", formatMoney(invoice.subtotal).c_str(),
                kTaxPercent, formatMoney(invoice.tax).c_str(), formatMoney(invoice.total).c_str());
    std::printf("  Paid %s | Change %s\n", formatMoney(invoice.paid).c_str(),
                formatMoney(invoice.change).c_str());
}

void printInvoiceTable(const std::vector<Invoice>& list) {
    std::printf("%-6s %-17s %6s %14s\n", "Inv #", "Finalized", "Items", "Total");
    Money grand = 0;
    for (const Invoice& invoice : list) {
        std::printf("%-6d %-17s %6d %14s\n", invoice.id, formatTime(invoice.finalizedAt).c_str(),
                    invoice.itemCount, formatMoney(invoice.total).c_str());
        grand += invoice.total;
    }
    std::printf("%zu invoice(s), grand total %s\n", list.size(), formatMoney(grand).c_str());
}

ProductPtr chooseProduct(const Catalog& catalog) {
    std::string query = prompt("Search item code or name (blank = done): ");
    if (query.empty()) {
        return nullptr;
    }
    std::vector<ProductPtr> found = catalog.search(query);
    if (found.empty()) {
        std::printf("No match\n");
        return nullptr;
    }
    if (found.size() == 1) {
        return found[0];
    }
    std::size_t shown = found.size() < kMaxChoices ? found.size() : kMaxChoices;
    for (std::size_t i = 0; i < shown; ++i) {
        std::printf("  %zu) %-9s %-55.55s %8.2f\n", i + 1, found[i]->code().c_str(),
                    found[i]->fullName().c_str(), found[i]->basePrice());
    }
    if (found.size() > shown) {
        std::printf("  ... %zu more, refine the search to see them\n", found.size() - shown);
    }
    int pick = readInt("Pick number (0 = cancel): ", 0, static_cast<int>(shown));
    return pick == 0 ? nullptr : found[static_cast<std::size_t>(pick) - 1];
}

PackType choosePack(const Product& product) {
    std::vector<PackType> packs = product.availablePacks();
    if (packs.size() == 1) {
        return packs[0];
    }
    for (std::size_t i = 0; i < packs.size(); ++i) {
        std::printf("  %zu) %-8s %10.2f\n", i + 1, packLabel(packs[i]).c_str(),
                    product.getPrice(packs[i]));
    }
    int pick = readInt("Pack type: ", 1, static_cast<int>(packs.size()));
    return packs[static_cast<std::size_t>(pick) - 1];
}

void addLine(Invoice& invoice, const Product& product, PackType pack, int quantity) {
    for (LineItem& existing : invoice.items) {
        if (existing.code == product.code() && existing.pack == pack) {
            existing.quantity += quantity;
            return;
        }
    }
    LineItem item;
    item.code = product.code();
    item.name = product.fullName();
    item.pack = pack;
    item.quantity = quantity;
    item.unitPrice = toCentavos(product.getPrice(pack));
    invoice.items.push_back(item);
}

void createInvoice(const Catalog& catalog, InvoicePriorityQueue& pending, int& nextId) {
    Invoice invoice;
    while (true) {
        ProductPtr product = chooseProduct(catalog);
        if (!product) {
            if (invoice.items.empty()) {
                std::printf("Invoice cancelled\n");
                return;
            }
            break;
        }
        PackType pack = choosePack(*product);
        int quantity = readInt("Quantity: ", 1, 100000);
        addLine(invoice, *product, pack, quantity);
        invoice.recompute();
        std::printf("Added. Running total: %s\n", formatMoney(invoice.total).c_str());
    }
    invoice.recompute();
    std::printf("Total due: %s\n", formatMoney(invoice.total).c_str());
    do {
        invoice.paid = readPesos("Amount paid: ");
        if (invoice.paid < invoice.total) {
            std::printf("Insufficient payment\n");
        }
    } while (invoice.paid < invoice.total);
    invoice.change = invoice.paid - invoice.total;
    invoice.id = nextId++;
    invoice.finalizedAt = std::time(nullptr);
    pending.enqueue(invoice);
    std::printf("Invoice #%d queued (%zu pending). Change: %s\n", invoice.id, pending.size(),
                formatMoney(invoice.change).c_str());
}

void loadSamples(const Catalog& catalog, InvoicePriorityQueue& pending, int& nextId) {
    std::vector<ProductPtr> all = catalog.getAllProducts();
    if (all.size() < 10) {
        std::printf("Catalog is too small for the sample invoices\n");
        return;
    }
    // Each row: first item position, quantity, second item position, quantity
    const int plan[5][4] = {{0, 5, 20, 2}, {40, 12, 60, 4}, {80, 3, 0, 0}, {100, 30, 120, 8},
                            {140, 10, 0, 0}};
    std::time_t now = std::time(nullptr);
    for (int i = 0; i < 5; ++i) {
        Invoice invoice;
        invoice.id = nextId++;
        invoice.finalizedAt = now - (5 - i) * 600;
        for (int part = 0; part < 2; ++part) {
            std::size_t index = static_cast<std::size_t>(plan[i][part * 2]);
            int quantity = plan[i][part * 2 + 1];
            if (quantity > 0 && index < all.size()) {
                addLine(invoice, *all[index], all[index]->defaultPack(), quantity);
            }
        }
        invoice.recompute();
        invoice.paid = invoice.total;
        pending.enqueue(invoice);
    }
    std::printf("Queued 5 sample invoices (%zu pending)\n", pending.size());
}

void showPending(const InvoicePriorityQueue& pending) {
    if (pending.isEmpty()) {
        std::printf("No pending invoices\n");
        return;
    }
    printInvoiceTable(pending.toPriorityList());
}

void processNext(InvoicePriorityQueue& pending, std::vector<Invoice>& log,
                 const std::string& logPath) {
    if (pending.isEmpty()) {
        std::printf("No pending invoices\n");
        return;
    }
    Invoice invoice = pending.dequeue();
    log.push_back(invoice);
    SalesLogStore::save(logPath, log);
    std::printf("Logged invoice #%d (total %s). %zu pending\n", invoice.id,
                formatMoney(invoice.total).c_str(), pending.size());
}

void showReport(const std::vector<Invoice>& log) {
    if (log.empty()) {
        std::printf("No logged sales yet\n");
        return;
    }
    std::printf("Sort by: 1) Date  2) Total value  3) Number of items\n");
    int choice = readInt("> ", 1, 3);
    std::vector<Invoice> sorted = log;
    quickSort(sorted, choice == 1 ? byDate : choice == 2 ? byTotalDesc : byItemCountDesc);
    printInvoiceTable(sorted);
}

void showAnalysis(const Catalog& catalog, const std::vector<Invoice>& log) {
    if (log.empty()) {
        std::printf("No logged sales yet\n");
        return;
    }
    std::vector<ProductStat> stats = aggregateSales(catalog, log);
    MinMaxResult r = findMinMax(stats);

    std::printf("%-9s %-50s %7s %14s\n", "Code", "Product", "Units", "Revenue");
    for (const ProductStat& s : stats) {
        if (s.unitsSold > 0) {
            std::printf("%-9s %-50.50s %7d %14s\n", s.code.c_str(), s.name.c_str(), s.unitsSold,
                        formatMoney(s.revenue).c_str());
        }
    }
    if (r.maxUnits >= 0) {
        std::printf("\nBest seller (units):    %s - %d\n", stats[r.maxUnits].name.c_str(),
                    stats[r.maxUnits].unitsSold);
        std::printf("Least sold (units):     %s - %d\n", stats[r.minUnits].name.c_str(),
                    stats[r.minUnits].unitsSold);
        std::printf("Highest revenue:        %s - %s\n", stats[r.maxRevenue].name.c_str(),
                    formatMoney(stats[r.maxRevenue].revenue).c_str());
        std::printf("Lowest revenue (sold):  %s - %s\n", stats[r.minRevenue].name.c_str(),
                    formatMoney(stats[r.minRevenue].revenue).c_str());
    }
    std::printf("Slow-moving (no sales): %zu product(s)\n", r.slowMoving.size());
    for (std::size_t i = 0; i < r.slowMoving.size() && i < kMaxSlowListed; ++i) {
        std::printf("  %-9s %s\n", stats[r.slowMoving[i]].code.c_str(),
                    stats[r.slowMoving[i]].name.c_str());
    }
    if (r.slowMoving.size() > kMaxSlowListed) {
        std::printf("  ... and %zu more\n", r.slowMoving.size() - kMaxSlowListed);
    }
}

void viewInvoice(const std::vector<Invoice>& log) {
    int id = readInt("Invoice #: ", 1, 100000000);
    for (const Invoice& invoice : log) {
        if (invoice.id == id) {
            printInvoice(invoice);
            return;
        }
    }
    std::printf("Invoice #%d is not in the sales log\n", id);
}

}

int main(int argc, char** argv) {
    std::string catalogPath = argc > 1 ? argv[1] : "data/products.csv";
    std::string logPath = argc > 2 ? argv[2] : "data/sales_log.csv";
    try {
        Catalog catalog = Catalog::fromCsv(catalogPath);
        std::vector<Invoice> log = SalesLogStore::load(logPath);
        InvoicePriorityQueue pending;
        int nextId = 1;
        for (const Invoice& invoice : log) {
            if (invoice.id >= nextId) {
                nextId = invoice.id + 1;
            }
        }
        std::printf("Loaded %zu catalog items, %zu logged invoices\n", catalog.size(), log.size());
        while (true) {
            std::cout << "\n1) New invoice  2) Load samples  3) Pending queue  4) Process next\n"
                         "5) Process all  6) Sales report  7) Product analysis  8) View invoice\n"
                         "0) Exit\n";
            std::string choice = prompt("> ");
            if (choice == "0") {
                break;
            }
            if (choice == "1") {
                createInvoice(catalog, pending, nextId);
            } else if (choice == "2") {
                loadSamples(catalog, pending, nextId);
            } else if (choice == "3") {
                showPending(pending);
            } else if (choice == "4") {
                processNext(pending, log, logPath);
            } else if (choice == "5") {
                if (pending.isEmpty()) {
                    std::printf("No pending invoices\n");
                }
                while (!pending.isEmpty()) {
                    processNext(pending, log, logPath);
                }
            } else if (choice == "6") {
                showReport(log);
            } else if (choice == "7") {
                showAnalysis(catalog, log);
            } else if (choice == "8") {
                viewInvoice(log);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
    return 0;
}
