#pragma once

#include <string>

#include "InvoiceList.h"
#include "Stack.h"

namespace jc8 {

// Optional hook for the product catalog owned by another module. The invoice
// module never changes stock itself; checkout only asks whether the requested
// quantities are available. The checker must outlive the manager.
class IStockChecker {
public:
    virtual ~IStockChecker() = default;

    virtual bool hasEnoughStock(const std::string& productCode,
                                PackageType package,
                                int quantity) const = 0;
};

// The result of a traversal over the active invoice.
struct PricingSummary {
    int lineCount = 0;
    int unitCount = 0;
    double subtotal = 0.0;
    double taxRate = 0.0;
    double tax = 0.0;
    double total = 0.0;

    // Positive means change due; negative means the cash is short.
    double changeFor(double cashTendered) const;
    bool isCoveredBy(double cashTendered) const;
};

struct CheckoutResult {
    bool success = false;
    std::string message;
    PricingSummary totals;
    double cashTendered = 0.0;
    double change = 0.0;

    // A copy of the finalized rows. After a successful checkout the active
    // invoice is empty, so this is what the receipt/queue module should use.
    InvoiceList items;
};

// Owns the active invoice (linked list), undo/redo stacks, traversal totals,
// and checkout. All methods are meant to be called from the UI thread.
class TransactionManager {
public:
    // taxRate is a fraction: 0.12 means 12% VAT. maxUndoDepth <= 0 means
    // unlimited history. Throws std::invalid_argument if taxRate is outside
    // [0, 1].
    explicit TransactionManager(double taxRate = 0.12, int maxUndoDepth = 100);

    // --- Active invoice commands (call these from the cashier UI) ---
    bool addItem(const Product& product, PackageType package, int quantity = 1);
    bool removeItem(int lineId);

    // Sets the row's quantity to an absolute value (not a delta).
    bool updateQuantity(int lineId, int quantity);

    // --- Undo / redo ---
    bool undo();
    bool redo();
    bool canUndo() const;
    bool canRedo() const;
    int undoCount() const;
    int redoCount() const;

    // --- Totals and checkout ---
    PricingSummary calculateTotal() const;

    // Negative when the cash tendered is less than the total.
    double calculateChange(double cashTendered) const;

    // On success, returns a copy of the finalized rows and clears the active
    // invoice and both histories. On failure, the invoice is unchanged.
    CheckoutResult checkout(double cashTendered);

    // Starts a blank invoice and clears the undo/redo histories.
    void clearInvoice();

    // --- Access for the Qt table model ---
    const InvoiceList& items() const;
    int lineCount() const;

    // --- Configuration ---
    double taxRate() const;
    bool setTaxRate(double taxRate);
    int maxUndoDepth() const;

    void setStockChecker(const IStockChecker* checker);
    const IStockChecker* stockChecker() const;

    // Human-readable reason for the most recent failed command. Empty after
    // a successful command.
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
