#pragma once

#include <string>

#include "InvoiceList.h"
#include "Stack.h"

namespace jc8 {

class IStockChecker {
public:
    virtual ~IStockChecker() = default;

    virtual bool hasEnoughStock(const std::string& productCode,
                                PackageType package,
                                int quantity) const = 0;
};

struct PricingSummary {
    int lineCount = 0;
    int unitCount = 0;
    double subtotal = 0.0;
    double taxRate = 0.0;
    double tax = 0.0;
    double total = 0.0;

    double changeFor(double cashTendered) const;
    bool isCoveredBy(double cashTendered) const;
};

struct CheckoutResult {
    bool success = false;
    std::string message;
    PricingSummary totals;
    double cashTendered = 0.0;
    double change = 0.0;

    InvoiceList items;
};

class TransactionManager {
public:
    explicit TransactionManager(double taxRate = 0.12, int maxUndoDepth = 100);
	
    bool addItem(const Product& product, PackageType package, int quantity = 1);
    bool removeItem(int lineId);

    bool updateQuantity(int lineId, int quantity);

    bool undo();
    bool redo();
    bool canUndo() const;
    bool canRedo() const;
    int undoCount() const;
    int redoCount() const;

    PricingSummary calculateTotal() const;

    double calculateChange(double cashTendered) const;

    CheckoutResult checkout(double cashTendered);

    void clearInvoice();

    // Access for the Qt table
    const InvoiceList& items() const;
    int lineCount() const;

    // Configuration
    double taxRate() const;
    bool setTaxRate(double taxRate);
    int maxUndoDepth() const;

    void setStockChecker(const IStockChecker* checker);
    const IStockChecker* stockChecker() const;

    // for error showing
    const std::string& lastError() const;

private:
    struct InvoiceState {
        InvoiceList items;
        int nextLineId = 1;
    };

    using UndoStack = Stack<InvoiceState>;
    using RedoStack = Stack<InvoiceState>;

    InvoiceList invoice_;
    UndoStack undoStack_;
    RedoStack redoStack_;
    int nextLineId_ = 1;
    int maxUndoDepth_ = 100;
    double taxRate_ = 0.12;
    const IStockChecker* stockChecker_ = nullptr;
    std::string lastError_;

    InvoiceState captureState() const;
    void recordUndoState();
    void resetInvoice();
    void setError(const std::string& message);
    bool checkStock(std::string& error) const;
};

} // namespace jc8
