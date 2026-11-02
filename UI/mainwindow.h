#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStack>
#include <QStringList>
#include <QList>
#include <QString>
#include <QMap>

struct Product {
    QString code;
    QString name;
    int stock;
    double piece;   // 0 means "not sold by piece"
    double roll;    // 0 means "not sold by roll"
    double box;
};

using Snapshot = QList<QStringList>;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onMenuButtonClicked();
    void onInvoiceButtonClicked();
    void onSalesButtonClicked();
    void onInventoryButtonClicked();
    void onAddClicked();
    void onRemoveClicked();
    void onUndoClicked();
    void onRedoClicked();
    void onSearchClicked();
    void onCheckoutClicked();

private:
    void addInvoiceRow(const QString &name, const QString &type, int qty, double lineTotal);
    void showTotals(double subtotal, double tax, double total);
    void tempRecalculate();

    void showSalesCards(double totalSales, const QString &bestName, int bestUnits,
                        const QString &leastName, int leastUnits);
    void addSalesRow(const QString &item, int units, const QString &status);

    void refreshSalesReport();
    QMap<QString, int> unitsSold;
    double salesTotal = 0;

    Snapshot takeSnapshot();
    void restoreSnapshot(const Snapshot &s);
    void saveForUndo();
    QStack<Snapshot> undoStack;
    QStack<Snapshot> redoStack;

    QList<Product> products;
    void loadProducts();
    void refreshCatalog();
    void refreshInventory();

    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
