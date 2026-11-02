#include "core/TransactionManager.h"

#include <cmath>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

int checks = 0;
int failures = 0;

void check(bool condition, const char* expression, const char* file, int line) {
    ++checks;
    if (!condition) {
        ++failures;
        std::cerr << file << ':' << line << ": CHECK failed: " << expression << '\n';
    }
}

void checkNear(double actual,
               double expected,
               const char* expression,
               const char* file,
               int line) {
    ++checks;
    if (std::fabs(actual - expected) > 1e-9) {
        ++failures;
        std::cerr << file << ':' << line << ": CHECK_NEAR failed: " << expression
                  << " (actual " << actual << ", expected " << expected << ")\n";
    }
}

#define CHECK(condition) check((condition), #condition, __FILE__, __LINE__)
#define CHECK_NEAR(actual, expected) \
    checkNear((actual), (expected), #actual, __FILE__, __LINE__)

jc8::InvoiceItem makeItem(int lineId,
                          const char* code,
                          jc8::PackageType package,
                          double price,
                          int quantity) {
    jc8::InvoiceItem item;
    item.lineId = lineId;
    item.productCode = code;
    item.description = code;
    item.package = package;
    item.unitPrice = price;
    item.quantity = quantity;
    return item;
}

void testStack() {
    jc8::Stack<int> stack;
    CHECK(stack.isEmpty());
    CHECK(stack.size() == 0);
    int value = 42;
    CHECK(!stack.tryPop(value));
    CHECK(value == 42);

    stack.push(1);
    stack.push(2);
    stack.push(3);
    CHECK(stack.size() == 3);
    CHECK(stack.top() == 3);
    CHECK(stack.tryPop(value));
    CHECK(value == 3);
    CHECK(stack.tryPop(value));
    CHECK(value == 2);
    CHECK(stack.tryPop(value));
    CHECK(value == 1);
    CHECK(stack.isEmpty());

    stack.push(10);
    stack.push(20);
    stack.push(30);
    stack.dropBottom();
    CHECK(stack.size() == 2);
    CHECK(stack.tryPop(value));
    CHECK(value == 30);
    CHECK(stack.tryPop(value));
    CHECK(value == 20);
    CHECK(stack.isEmpty());
    stack.dropBottom();
    CHECK(stack.isEmpty());

    jc8::Stack<int> source;
    source.push(7);
    source.push(8);
    jc8::Stack<int> copied(source);
    CHECK(copied.size() == 2);
    CHECK(copied.tryPop(value));
    CHECK(value == 8);
    CHECK(copied.tryPop(value));
    CHECK(value == 7);
    CHECK(source.size() == 2);

    jc8::Stack<int> assigned;
    assigned.push(100);
    assigned = source;
    CHECK(assigned.size() == 2);
    CHECK(assigned.tryPop(value));
    CHECK(value == 8);

    jc8::Stack<int> moved(std::move(assigned));
    CHECK(moved.size() == 1);
    CHECK(moved.tryPop(value));
    CHECK(value == 7);
    CHECK(assigned.isEmpty());
}

void testInvoiceList() {
    jc8::InvoiceList list;
    CHECK(list.isEmpty());
    CHECK(list.size() == 0);
    CHECK(list.begin() == list.end());
    CHECK(list.itemAt(0) == nullptr);
    CHECK(list.findById(1) == nullptr);
    CHECK(list.indexOf(1) == -1);
    CHECK(!list.removeById(1));

    list.append(makeItem(1, "A", jc8::PackageType::Set, 10.0, 2));
    list.append(makeItem(2, "B", jc8::PackageType::Rolls, 20.0, 1));
    list.append(makeItem(3, "C", jc8::PackageType::Box, 30.0, 4));
    CHECK(list.size() == 3);

    int expectedId = 1;
    for (const jc8::InvoiceItem& item : list) {
        CHECK(item.lineId == expectedId);
        ++expectedId;
    }
    CHECK(expectedId == 4);

    CHECK(list.itemAt(0) != nullptr);
    CHECK(list.itemAt(0)->productCode == "A");
    CHECK(list.itemAt(2) != nullptr);
    CHECK(list.itemAt(3) == nullptr);
    CHECK(list.itemAt(-1) == nullptr);
    CHECK(list.findById(2) != nullptr);
    CHECK(list.findById(2)->quantity == 1);
    CHECK(list.indexOf(3) == 2);
    CHECK(list.indexOf(99) == -1);

    jc8::InvoiceList copy(list);
    CHECK(copy.size() == 3);
    jc8::InvoiceItem* copyFirst = copy.findById(1);
    CHECK(copyFirst != nullptr);
    if (copyFirst != nullptr) {
        copyFirst->quantity = 99;
    }
    const jc8::InvoiceItem* listFirst = list.findById(1);
    const jc8::InvoiceItem* copyFirstAgain = copy.findById(1);
    CHECK(listFirst != nullptr && listFirst->quantity == 2);
    CHECK(copyFirstAgain != nullptr && copyFirstAgain->quantity == 99);

    jc8::InvoiceList assigned;
    assigned = list;
    CHECK(assigned.size() == 3);
    assigned.removeById(2);
    CHECK(assigned.size() == 2);
    CHECK(list.size() == 3);

    jc8::InvoiceList moved(std::move(copy));
    CHECK(moved.size() == 3);
    CHECK(copy.isEmpty());

    CHECK(list.removeById(2));
    CHECK(list.size() == 2);
    CHECK(list.findById(2) == nullptr);
    CHECK(list.itemAt(1) != nullptr);
    CHECK(list.itemAt(1)->lineId == 3);

    CHECK(list.removeById(3));
    CHECK(list.size() == 1);
    CHECK(list.findById(1) != nullptr);

    // Append after removing the tail must reconnect the tail pointer.
    list.append(makeItem(4, "D", jc8::PackageType::Set, 40.0, 1));
    CHECK(list.size() == 2);
    CHECK(list.itemAt(1) != nullptr);
    CHECK(list.itemAt(1)->lineId == 4);
    CHECK(list.removeById(4));
    CHECK(list.size() == 1);

    CHECK(list.removeById(1));
    CHECK(list.isEmpty());
    CHECK(list.begin() == list.end());

    list.append(makeItem(9, "Z", jc8::PackageType::Box, 1.0, 1));
    CHECK(list.size() == 1);
    list.clear();
    CHECK(list.isEmpty());
}

void testPackageTypesAndPricing() {
    CHECK(std::string(jc8::packageTypeName(jc8::PackageType::Set)) == "Set");
    CHECK(std::string(jc8::packageTypeName(jc8::PackageType::Rolls)) == "Rolls");
    CHECK(std::string(jc8::packageTypeName(jc8::PackageType::Box)) == "Box");

    jc8::PackageType parsed = jc8::PackageType::Set;
    CHECK(jc8::packageTypeFromString("ROLLS", parsed));
    CHECK(parsed == jc8::PackageType::Rolls);
    CHECK(jc8::packageTypeFromString("box", parsed));
    CHECK(parsed == jc8::PackageType::Box);
    CHECK(!jc8::packageTypeFromString("crate", parsed));

    CHECK(jc8::isValidPackageType(jc8::PackageType::Set));
    CHECK(jc8::isValidPackageType(jc8::PackageType::Rolls));
    CHECK(jc8::isValidPackageType(jc8::PackageType::Box));
    CHECK(!jc8::isValidPackageType(static_cast<jc8::PackageType>(99)));
    CHECK(!jc8::isValidPackageType(static_cast<jc8::PackageType>(-1)));

    const jc8::Product product("SKU-1", "Steel Rack", 100.0, 10.0, 120.0);
    CHECK_NEAR(product.priceFor(jc8::PackageType::Set), 100.0);
    CHECK_NEAR(product.priceFor(jc8::PackageType::Rolls), 10.0);
    CHECK_NEAR(product.priceFor(jc8::PackageType::Box), 120.0);
    CHECK_NEAR(product.priceFor(static_cast<jc8::PackageType>(99)), 0.0);
}

void testActiveInvoiceAndTotals() {
    jc8::TransactionManager manager(0.12);
    CHECK_NEAR(manager.taxRate(), 0.12);

    const jc8::Product product("SKU-1", "Steel Rack", 100.0, 10.0, 120.0);
    CHECK(manager.addItem(product, jc8::PackageType::Set, 2));
    CHECK(manager.addItem(product, jc8::PackageType::Rolls, 3));
    CHECK(manager.addItem(product, jc8::PackageType::Box, 1));
    CHECK(manager.lineCount() == 3);
    CHECK(manager.items().size() == 3);
    CHECK(manager.undoCount() == 3);
    CHECK(manager.lastError().empty());

    const jc8::InvoiceItem* first = manager.items().itemAt(0);
    CHECK(first != nullptr);
    if (first != nullptr) {
        CHECK(first->lineId == 1);
        CHECK(first->productCode == "SKU-1");
        CHECK(first->description == "Steel Rack");
        CHECK(first->package == jc8::PackageType::Set);
        CHECK_NEAR(first->unitPrice, 100.0);
        CHECK(first->quantity == 2);
        CHECK_NEAR(first->lineTotal(), 200.0);
    }

    const jc8::InvoiceItem* second = manager.items().itemAt(1);
    CHECK(second != nullptr);
    if (second != nullptr) {
        CHECK(second->lineId == 2);
        CHECK_NEAR(second->unitPrice, 10.0);
    }

    const jc8::InvoiceItem* third = manager.items().itemAt(2);
    CHECK(third != nullptr);
    if (third != nullptr) {
        CHECK(third->lineId == 3);
        CHECK_NEAR(third->unitPrice, 120.0);
    }

    const jc8::PricingSummary totals = manager.calculateTotal();
    CHECK(totals.lineCount == 3);
    CHECK(totals.unitCount == 6);
    CHECK_NEAR(totals.subtotal, 350.0);
    CHECK_NEAR(totals.taxRate, 0.12);
    CHECK_NEAR(totals.tax, 42.0);
    CHECK_NEAR(totals.total, 392.0);
    CHECK(totals.isCoveredBy(392.0));
    CHECK(!totals.isCoveredBy(391.99));
    CHECK_NEAR(totals.changeFor(400.0), 8.0);
    CHECK_NEAR(totals.changeFor(300.0), -92.0);
    CHECK_NEAR(manager.calculateChange(400.0), 8.0);
    CHECK_NEAR(manager.calculateChange(300.0), -92.0);

    const int undoBefore = manager.undoCount();
    CHECK(!manager.addItem(product, jc8::PackageType::Set, 0));
    CHECK(!manager.lastError().empty());
    CHECK(manager.undoCount() == undoBefore);
    CHECK(manager.lineCount() == 3);

    CHECK(!manager.addItem(jc8::Product("", "No code", 1, 1, 1),
                           jc8::PackageType::Set, 1));
    CHECK(!manager.addItem(jc8::Product("SKU-X", "Bad price", -1, 1, 1),
                           jc8::PackageType::Set, 1));
    CHECK(!manager.addItem(product, static_cast<jc8::PackageType>(99), 1));
    CHECK(manager.lineCount() == 3);

    // Free items are valid; the total should stay at zero.
    CHECK(manager.addItem(jc8::Product("FREE-1", "Sample", 0, 0, 0),
                          jc8::PackageType::Set, 1));
    CHECK_NEAR(manager.calculateTotal().total, 392.0);
}

void testRounding() {
    jc8::TransactionManager manager(0.12);
    const jc8::Product product("SKU-ROUND", "Rounding Test", 19.99, 19.99, 19.99);
    CHECK(manager.addItem(product, jc8::PackageType::Set, 3));

    const jc8::PricingSummary totals = manager.calculateTotal();
    CHECK_NEAR(totals.subtotal, 59.97);
    CHECK_NEAR(totals.tax, 7.20);
    CHECK_NEAR(totals.total, 67.17);
    CHECK_NEAR(manager.calculateChange(100.0), 32.83);
}

void testQuantityRemoveUndoRedo() {
    jc8::TransactionManager manager(0.12);
    const jc8::Product product("SKU-1", "Widget", 100.0, 10.0, 120.0);

    CHECK(manager.addItem(product, jc8::PackageType::Set, 1));
    CHECK(manager.addItem(product, jc8::PackageType::Rolls, 2));
    CHECK(manager.lineCount() == 2);
    CHECK_NEAR(manager.calculateTotal().subtotal, 120.0);

    CHECK(manager.updateQuantity(1, 3));
    CHECK_NEAR(manager.calculateTotal().subtotal, 320.0);
    const jc8::InvoiceItem* line = manager.items().findById(1);
    CHECK(line != nullptr && line->quantity == 3);

    CHECK(manager.undo());
    line = manager.items().findById(1);
    CHECK(line != nullptr && line->quantity == 1);
    CHECK_NEAR(manager.calculateTotal().subtotal, 120.0);

    CHECK(manager.redo());
    line = manager.items().findById(1);
    CHECK(line != nullptr && line->quantity == 3);
    CHECK_NEAR(manager.calculateTotal().subtotal, 320.0);

    CHECK(manager.removeItem(2));
    CHECK(manager.lineCount() == 1);
    CHECK_NEAR(manager.calculateTotal().subtotal, 300.0);

    CHECK(manager.undo());
    CHECK(manager.lineCount() == 2);
    CHECK_NEAR(manager.calculateTotal().subtotal, 320.0);
    CHECK(manager.canRedo());

    CHECK(manager.redo());
    CHECK(manager.lineCount() == 1);
    CHECK_NEAR(manager.calculateTotal().subtotal, 300.0);

    // A fresh action after undo must discard the redo branch.
    CHECK(manager.undo());
    CHECK(manager.canRedo());
    CHECK(manager.addItem(product, jc8::PackageType::Box, 1));
    CHECK(!manager.canRedo());
    CHECK(manager.lineCount() == 3);

    // Line IDs must not be reused after undoing and redoing an add.
    CHECK(manager.undo());
    CHECK(manager.redo());
    CHECK(manager.items().findById(3) != nullptr);
    CHECK(manager.addItem(product, jc8::PackageType::Set, 1));
    CHECK(manager.items().findById(4) != nullptr);
    CHECK(manager.lineCount() == 4);

    const int undoBeforeNoOp = manager.undoCount();
    CHECK(manager.updateQuantity(1, 3));
    CHECK(manager.undoCount() == undoBeforeNoOp);
    CHECK(manager.lastError().empty());

    CHECK(!manager.removeItem(999));
    CHECK(!manager.lastError().empty());
    CHECK(!manager.updateQuantity(999, 2));
    CHECK(!manager.updateQuantity(1, 0));
    CHECK(manager.lineCount() == 4);

    CHECK(!manager.canRedo());
    while (manager.canUndo()) {
        CHECK(manager.undo());
    }
    CHECK(manager.lineCount() == 0);
    CHECK(!manager.undo());
    CHECK(!manager.lastError().empty());
}

void testUndoDepthLimit() {
    jc8::TransactionManager manager(0.12, 2);
    CHECK(manager.maxUndoDepth() == 2);
    const jc8::Product product("SKU-1", "Widget", 100.0, 10.0, 120.0);
    CHECK(manager.addItem(product, jc8::PackageType::Set, 1));
    CHECK(manager.addItem(product, jc8::PackageType::Set, 1));
    CHECK(manager.addItem(product, jc8::PackageType::Set, 1));
    CHECK(manager.undoCount() == 2);

    CHECK(manager.undo());
    CHECK(manager.redoCount() == 1);
    CHECK(manager.undo());
    CHECK(manager.redoCount() == 2);
    CHECK(manager.lineCount() == 1);
    CHECK(!manager.undo());
    CHECK(manager.lastError() == "There is nothing to undo.");

    jc8::TransactionManager unlimited(0.12, 0);
    CHECK(unlimited.addItem(product, jc8::PackageType::Set, 1));
    CHECK(unlimited.addItem(product, jc8::PackageType::Set, 1));
    CHECK(unlimited.addItem(product, jc8::PackageType::Set, 1));
    CHECK(unlimited.undoCount() == 3);
}

void testCheckout() {
    jc8::TransactionManager manager(0.12);
    const jc8::Product product("SKU-1", "Kettle", 100.0, 50.0, 200.0);
    CHECK(manager.addItem(product, jc8::PackageType::Set, 2));

    CHECK_NEAR(manager.calculateTotal().subtotal, 200.0);
    CHECK_NEAR(manager.calculateTotal().tax, 24.0);
    CHECK_NEAR(manager.calculateTotal().total, 224.0);
    CHECK_NEAR(manager.calculateChange(250.0), 26.0);
    CHECK_NEAR(manager.calculateChange(200.0), -24.0);

    jc8::CheckoutResult failed = manager.checkout(200.0);
    CHECK(!failed.success);
    CHECK(failed.message == "Insufficient payment.");
    CHECK_NEAR(failed.totals.total, 224.0);
    CHECK_NEAR(failed.change, 0.0);
    CHECK(manager.lineCount() == 1);
    CHECK(manager.canUndo());

    jc8::CheckoutResult invalidCash = manager.checkout(
        std::numeric_limits<double>::quiet_NaN());
    CHECK(!invalidCash.success);
    CHECK(manager.lineCount() == 1);

    jc8::CheckoutResult negativeCash = manager.checkout(-1.0);
    CHECK(!negativeCash.success);
    CHECK(manager.lineCount() == 1);

    jc8::CheckoutResult success = manager.checkout(250.0);
    CHECK(success.success);
    CHECK(success.message == "Payment accepted.");
    CHECK_NEAR(success.totals.subtotal, 200.0);
    CHECK_NEAR(success.totals.tax, 24.0);
    CHECK_NEAR(success.totals.total, 224.0);
    CHECK_NEAR(success.cashTendered, 250.0);
    CHECK_NEAR(success.change, 26.0);
    CHECK(success.items.size() == 1);
    const jc8::InvoiceItem* checkedOut = success.items.findById(1);
    CHECK(checkedOut != nullptr && checkedOut->quantity == 2);

    CHECK(manager.lineCount() == 0);
    CHECK(!manager.canUndo());
    CHECK(!manager.canRedo());
    CHECK(manager.lastError().empty());

    jc8::CheckoutResult empty = manager.checkout(100.0);
    CHECK(!empty.success);
    CHECK(empty.message == "Cannot check out an empty invoice.");

    CHECK(manager.addItem(product, jc8::PackageType::Set, 1));
    const jc8::InvoiceItem* firstNew = manager.items().itemAt(0);
    CHECK(firstNew != nullptr && firstNew->lineId == 1);
    jc8::CheckoutResult exact = manager.checkout(112.0);
    CHECK(exact.success);
    CHECK_NEAR(exact.change, 0.0);
    CHECK(manager.lineCount() == 0);
}

void testTaxConfigurationAndClear() {
    jc8::TransactionManager manager(0.0);
    const jc8::Product product("SKU-1", "Kettle", 50.0, 50.0, 50.0);
    CHECK(manager.addItem(product, jc8::PackageType::Set, 2));
    CHECK_NEAR(manager.calculateTotal().tax, 0.0);
    CHECK_NEAR(manager.calculateTotal().total, 100.0);

    CHECK(manager.setTaxRate(0.12));
    CHECK_NEAR(manager.calculateTotal().total, 112.0);
    CHECK(!manager.setTaxRate(1.5));
    CHECK_NEAR(manager.taxRate(), 0.12);
    CHECK(!manager.setTaxRate(std::numeric_limits<double>::quiet_NaN()));
    CHECK_NEAR(manager.taxRate(), 0.12);

    manager.clearInvoice();
    CHECK(manager.lineCount() == 0);
    CHECK(!manager.canUndo());
    CHECK(!manager.canRedo());
    CHECK_NEAR(manager.taxRate(), 0.12);

    bool threw = false;
    try {
        jc8::TransactionManager invalid(-0.1);
        (void)invalid;
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    CHECK(threw);
}

struct FakeStockChecker : jc8::IStockChecker {
    mutable int calls = 0;
    mutable int lastQuantity = 0;
    int available = 4;

    bool hasEnoughStock(const std::string& productCode,
                        jc8::PackageType package,
                        int quantity) const override {
        (void)productCode;
        (void)package;
        ++calls;
        lastQuantity = quantity;
        return quantity <= available;
    }
};

void testStockChecker() {
    FakeStockChecker checker;
    jc8::TransactionManager manager(0.12);
    manager.setStockChecker(&checker);
    CHECK(manager.stockChecker() == &checker);

    const jc8::Product product("STK-1", "Tape", 20.0, 5.0, 40.0);
    CHECK(manager.addItem(product, jc8::PackageType::Rolls, 2));
    CHECK(manager.addItem(product, jc8::PackageType::Rolls, 3));

    jc8::CheckoutResult failed = manager.checkout(1000.0);
    CHECK(!failed.success);
    CHECK(checker.calls == 1);
    CHECK(checker.lastQuantity == 5);
    CHECK(failed.message.find("Insufficient stock") != std::string::npos);
    CHECK(manager.lineCount() == 2);
    CHECK(manager.canUndo());

    CHECK(manager.updateQuantity(2, 1));
    jc8::CheckoutResult success = manager.checkout(1000.0);
    CHECK(success.success);
    CHECK(checker.calls == 2);
    CHECK(checker.lastQuantity == 3);
    CHECK(success.items.size() == 2);
    CHECK(manager.lineCount() == 0);

    manager.setStockChecker(nullptr);
    CHECK(manager.stockChecker() == nullptr);
}

void runTest(const char* name, void (*test)()) {
    try {
        test();
        std::cout << "[PASS] " << name << '\n';
    } catch (const std::exception& exception) {
        ++failures;
        std::cerr << "[FAIL] " << name << ": unexpected exception: "
                  << exception.what() << '\n';
    } catch (...) {
        ++failures;
        std::cerr << "[FAIL] " << name << ": unexpected unknown exception\n";
    }
}

} // namespace

int main() {
    runTest("stack", testStack);
    runTest("invoice list", testInvoiceList);
    runTest("package types and pricing", testPackageTypesAndPricing);
    runTest("active invoice and totals", testActiveInvoiceAndTotals);
    runTest("rounding", testRounding);
    runTest("quantity/remove/undo/redo", testQuantityRemoveUndoRedo);
    runTest("undo depth limit", testUndoDepthLimit);
    runTest("checkout", testCheckout);
    runTest("tax configuration and clear", testTaxConfigurationAndClear);
    runTest("stock checker", testStockChecker);

    std::cout << '\n' << checks << " checks, " << failures << " failure(s).\n";
    return failures == 0 ? 0 : 1;
}
