#include "TransactionManager.h"

#include <cmath>
#include <stdexcept>
#include <utility>

namespace jc8 {

double PricingSummary::changeFor(double cashTendered) const {
    return roundMoney(cashTendered - total);
}

bool PricingSummary::isCoveredBy(double cashTendered) const {
    return std::isfinite(cashTendered) &&
           roundMoney(cashTendered) + 1e-9 >= total;
}

TransactionManager::TransactionManager(double taxRate, int maxUndoDepth)
    : maxUndoDepth_(maxUndoDepth < 0 ? 0 : maxUndoDepth) {
    if (!std::isfinite(taxRate) || taxRate < 0.0 || taxRate > 1.0) {
        throw std::invalid_argument("Tax rate must be between 0 and 1.");
    }
    taxRate_ = taxRate;
}

bool TransactionManager::addItem(const Product& product,
                                 PackageType package,
                                 int quantity) {
    lastError_.clear();

    if (!isValidPackageType(package)) {
        setError("Invalid package type.");
        return false;
    }
    if (quantity <= 0) {
        setError("Quantity must be at least 1.");
        return false;
    }
    if (product.code.empty()) {
        setError("Product code must not be empty.");
        return false;
    }

    const double price = product.priceFor(package);
    if (!std::isfinite(price) || price < 0.0) {
        setError("Unit price must be a non-negative number.");
        return false;
    }

    InvoiceItem item;
    item.lineId = nextLineId_;
    item.productCode = product.code;
    item.description = product.name.empty() ? product.code : product.name;
    item.package = package;
    item.unitPrice = roundMoney(price);
    item.quantity = quantity;

    recordUndoState();
    invoice_.append(item);
    ++nextLineId_;
    return true;
}

bool TransactionManager::removeItem(int lineId) {
    lastError_.clear();

    if (invoice_.findById(lineId) == nullptr) {
        setError("No invoice line has ID " + std::to_string(lineId) + ".");
        return false;
    }

    recordUndoState();
    invoice_.removeById(lineId);
    return true;
}

bool TransactionManager::updateQuantity(int lineId, int quantity) {
    lastError_.clear();

    if (quantity <= 0) {
        setError("Quantity must be at least 1. Use removeItem() to delete a line.");
        return false;
    }

    InvoiceItem* item = invoice_.findById(lineId);
    if (item == nullptr) {
        setError("No invoice line has ID " + std::to_string(lineId) + ".");
        return false;
    }
    if (item->quantity == quantity) {
        return true;
    }

    recordUndoState();
    item->quantity = quantity;
    return true;
}

bool TransactionManager::undo() {
    lastError_.clear();

    if (undoStack_.isEmpty()) {
        setError("There is nothing to undo.");
        return false;
    }

    redoStack_.push(captureState());

    InvoiceState previous;
    if (!undoStack_.tryPop(previous)) {
        setError("There is nothing to undo.");
        return false;
    }

    invoice_ = std::move(previous.items);
    nextLineId_ = previous.nextLineId;
    return true;
}

bool TransactionManager::redo() {
    lastError_.clear();

    if (redoStack_.isEmpty()) {
        setError("There is nothing to redo.");
        return false;
    }

    undoStack_.push(captureState());

    InvoiceState next;
    if (!redoStack_.tryPop(next)) {
        setError("There is nothing to redo.");
        return false;
    }

    invoice_ = std::move(next.items);
    nextLineId_ = next.nextLineId;
    return true;
}

bool TransactionManager::canUndo() const {
    return !undoStack_.isEmpty();
}

bool TransactionManager::canRedo() const {
    return !redoStack_.isEmpty();
}

int TransactionManager::undoCount() const {
    return undoStack_.size();
}

int TransactionManager::redoCount() const {
    return redoStack_.size();
}

PricingSummary TransactionManager::calculateTotal() const {
    PricingSummary summary;
    summary.taxRate = taxRate_;

    // traversal total
    for (const InvoiceItem& item : invoice_) {
        summary.subtotal += item.lineTotal();
        summary.unitCount += item.quantity;
        ++summary.lineCount;
    }

    summary.subtotal = roundMoney(summary.subtotal);
    summary.tax = roundMoney(summary.subtotal * taxRate_);
    summary.total = roundMoney(summary.subtotal + summary.tax);
    return summary;
}

double TransactionManager::calculateChange(double cashTendered) const {
    return calculateTotal().changeFor(cashTendered);
}

CheckoutResult TransactionManager::checkout(double cashTendered) {
    lastError_.clear();

    CheckoutResult result;
    result.totals = calculateTotal();
    result.cashTendered = roundMoney(cashTendered);

    if (invoice_.isEmpty()) {
        setError("Cannot check out an empty invoice.");
        result.message = lastError_;
        return result;
    }

    if (!std::isfinite(cashTendered) || cashTendered < 0.0) {
        setError("Cash tendered must be a non-negative number.");
        result.message = lastError_;
        return result;
    }

    if (!result.totals.isCoveredBy(cashTendered)) {
        setError("Insufficient payment.");
        result.message = lastError_;
        return result;
    }

    std::string stockError;
    if (!checkStock(stockError)) {
        setError(stockError);
        result.message = lastError_;
        return result;
    }

    result.success = true;
    result.message = "Payment accepted.";
    result.change = result.totals.changeFor(result.cashTendered);
    result.items = invoice_;

    resetInvoice();
    return result;
}

void TransactionManager::clearInvoice() {
    lastError_.clear();
    resetInvoice();
}

const InvoiceList& TransactionManager::items() const {
    return invoice_;
}

int TransactionManager::lineCount() const {
    return invoice_.size();
}

double TransactionManager::taxRate() const {
    return taxRate_;
}

bool TransactionManager::setTaxRate(double taxRate) {
    lastError_.clear();

    if (!std::isfinite(taxRate) || taxRate < 0.0 || taxRate > 1.0) {
        setError("Tax rate must be between 0 and 1.");
        return false;
    }
    taxRate_ = taxRate;
    return true;
}

int TransactionManager::maxUndoDepth() const {
    return maxUndoDepth_;
}

void TransactionManager::setStockChecker(const IStockChecker* checker) {
    lastError_.clear();
    stockChecker_ = checker;
}

const IStockChecker* TransactionManager::stockChecker() const {
    return stockChecker_;
}

const std::string& TransactionManager::lastError() const {
    return lastError_;
}

TransactionManager::InvoiceState TransactionManager::captureState() const {
    InvoiceState state;
    state.items = invoice_;
    state.nextLineId = nextLineId_;
    return state;
}

void TransactionManager::recordUndoState() {
    undoStack_.push(captureState());
    if (maxUndoDepth_ > 0) {
        while (undoStack_.size() > maxUndoDepth_) {
            undoStack_.dropBottom();
        }
    }
    redoStack_.clear();
}

void TransactionManager::resetInvoice() {
    invoice_.clear();
    undoStack_.clear();
    redoStack_.clear();
    nextLineId_ = 1;
}

void TransactionManager::setError(const std::string& message) {
    lastError_ = message;
}

bool TransactionManager::checkStock(std::string& error) const {
    if (stockChecker_ == nullptr) {
        return true;
    }

    for (const InvoiceItem& item : invoice_) {
        bool seenEarlier = false;
        for (const InvoiceItem& other : invoice_) {
            if (other.lineId == item.lineId) {
                break;
            }
            if (other.productCode == item.productCode && other.package == item.package) {
                seenEarlier = true;
                break;
            }
        }
        if (seenEarlier) {
            continue;
        }

        int requested = 0;
        for (const InvoiceItem& other : invoice_) {
            if (other.productCode == item.productCode && other.package == item.package) {
                requested += other.quantity;
            }
        }

        if (!stockChecker_->hasEnoughStock(item.productCode, item.package, requested)) {
            error = "Insufficient stock for " + item.description + " (" +
                    packageTypeName(item.package) + ").";
            return false;
        }
    }
    return true;
}

} // namespace jc8
