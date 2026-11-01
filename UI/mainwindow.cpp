#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QVBoxLayout>
#include <QDebug>
#include <QPushButton>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QPair>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /* Menu, New Invoice, Sales Report, and Inventory Buttons */
    ui->pagesStack->setCurrentIndex(0);
    connect(ui->menuButton, &QPushButton::clicked, this, &MainWindow::onMenuButtonClicked);
    connect(ui->invoiceButton, &QPushButton::clicked, this, &MainWindow::onInvoiceButtonClicked);
    connect(ui->salesButton, &QPushButton::clicked, this, &MainWindow::onSalesButtonClicked);
    connect(ui->inventoryButton, &QPushButton::clicked, this, &MainWindow::onInventoryButtonClicked);

    connect(ui->removeButton, &QPushButton::clicked, this, &MainWindow::onRemoveClicked);

    connect(ui->undoButton, &QPushButton::clicked, this, &MainWindow::onUndoClicked);
    connect(ui->redoButton, &QPushButton::clicked, this, &MainWindow::onRedoClicked);

    connect(ui->searchButton, &QPushButton::clicked, this, &MainWindow::onSearchClicked);
    connect(ui->searchBox, &QLineEdit::returnPressed, this, &MainWindow::onSearchClicked);

    connect(ui->checkoutButton, &QPushButton::clicked, this, &MainWindow::onCheckoutClicked);

    /* Page Layout*/
    QVBoxLayout *invLayout = new QVBoxLayout(ui->pageInventory);
    invLayout->addWidget(ui->inventoryTable);

    QHBoxLayout *invPageLayout = new QHBoxLayout(ui->pageInvoice);
    invPageLayout->addWidget(ui->leftPanel, 2);
    invPageLayout->addWidget(ui->rightPanel, 1);

    QVBoxLayout *salesLayout = new QVBoxLayout(ui->pageSales);
    salesLayout->addWidget(ui->cardsRow);
    salesLayout->addWidget(ui->movementLabel);
    salesLayout->addWidget(ui->salesTable);

    QVBoxLayout *menuLayout = new QVBoxLayout(ui->pageMenu);
    menuLayout->addStretch();
    menuLayout->addWidget(ui->titleLabel, 0, Qt::AlignHCenter);
    menuLayout->addWidget(ui->subtitleLabel, 0, Qt::AlignHCenter);
    menuLayout->addStretch();

    /* Inventory */
    ui->inventoryTable->setColumnCount(3);
    ui->inventoryTable->setHorizontalHeaderLabels({"Item", "Qty", "Price"});

    loadProducts();
    refreshCatalog();
    refreshInventory();

    /* Add Buttons */
    for (int i = 0; i < 4; i++) {
        QPushButton *addBtn = new QPushButton("Add");
        addBtn->setStyleSheet("background-color: #1f4e96; color: white; padding: 4px;");
        ui->catalogTable->setCellWidget(i, 5, addBtn);

        addBtn->setProperty("row", i);
        connect(addBtn, &QPushButton::clicked, this, &MainWindow::onAddClicked);
    }

    /* Sales Table */
    ui->salesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->salesTable->verticalHeader()->setVisible(false);
    ui->salesTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->salesTable->setSelectionMode(QAbstractItemView::NoSelection);
    ui->salesTable->setFocusPolicy(Qt::NoFocus);

    // Inventory table: read-only, even columns, no row numbers
    ui->inventoryTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->inventoryTable->verticalHeader()->setVisible(false);
    ui->inventoryTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Catalog table: same, but rows stay clickable
    ui->catalogTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->catalogTable->verticalHeader()->setVisible(false);
    ui->catalogTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->catalogTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    // Invoice table: same, rows selectable so Remove Item can use them later
    ui->invoiceTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->invoiceTable->verticalHeader()->setVisible(false);
    ui->invoiceTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->invoiceTable->setSelectionBehavior(QAbstractItemView::SelectRows);

refreshSalesReport();
}

/*Inventory*/

void MainWindow::loadProducts()
{
    products.clear();
}

void MainWindow::refreshCatalog()
{
    ui->catalogTable->setRowCount(products.size());
    for (int i = 0; i < products.size(); i++) {
        const Product &p = products[i];
        auto price = [](double v) { return v > 0 ? QString::number(v, 'f', 2) : QString("-"); };

        ui->catalogTable->setItem(i, 0, new QTableWidgetItem(p.code));
        ui->catalogTable->setItem(i, 1, new QTableWidgetItem(p.name));
        ui->catalogTable->setItem(i, 2, new QTableWidgetItem(price(p.piece)));
        ui->catalogTable->setItem(i, 3, new QTableWidgetItem(price(p.roll)));
        ui->catalogTable->setItem(i, 4, new QTableWidgetItem(price(p.box)));

        QPushButton *addBtn = new QPushButton("Add");
        addBtn->setStyleSheet("background-color: #1f4e96; color: white; padding: 4px;");
        addBtn->setProperty("row", i);
        connect(addBtn, &QPushButton::clicked, this, &MainWindow::onAddClicked);
        ui->catalogTable->setCellWidget(i, 5, addBtn);
    }
}

void MainWindow::refreshInventory()
{
    ui->inventoryTable->setRowCount(products.size());
    for (int i = 0; i < products.size(); i++) {
        const Product &p = products[i];
        double price = p.piece > 0 ? p.piece : p.roll;

        ui->inventoryTable->setItem(i, 0, new QTableWidgetItem(p.name));
        ui->inventoryTable->setItem(i, 1, new QTableWidgetItem(QString::number(p.stock)));
        ui->inventoryTable->setItem(i, 2, new QTableWidgetItem(QString::number(price, 'f', 2)));
    }
}

/* Checkout*/
void MainWindow::onCheckoutClicked()
{
    int rows = ui->invoiceTable->rowCount();
    if (rows == 0) {
        QMessageBox::information(this, "Checkout", "The invoice is empty.");
        return;
    }

    QMap<QString, int> wanted;
    double saleAmount = 0;
    for (int r = 0; r < rows; r++) {
        QString name = ui->invoiceTable->item(r, 0)->text();
        wanted[name] += ui->invoiceTable->item(r, 2)->text().toInt();
        saleAmount += ui->invoiceTable->item(r, 3)->text().toDouble();
    }

    for (auto it = wanted.begin(); it != wanted.end(); ++it) {
        for (const Product &p : products) {
            if (p.name == it.key() && p.stock < it.value()) {
                QMessageBox::warning(this, "Checkout",
                                     "Not enough stock for " + p.name + ".\nIn stock: " + QString::number(p.stock));
                return;
            }
        }
    }

    /* Records the unit sold */
    for (auto it = wanted.begin(); it != wanted.end(); ++it) {
        for (Product &p : products) {
            if (p.name == it.key()) {
                p.stock -= it.value();
                unitsSold[p.name] += it.value();
            }
        }
    }
    salesTotal += saleAmount;

    QMessageBox::information(this, "Checkout", "Sale completed.\n" + ui->totalLabel->text());

    /* Clears the invoice */
    ui->invoiceTable->setRowCount(0);
    undoStack.clear();
    redoStack.clear();
    tempRecalculate();
    refreshInventory();
    refreshSalesReport();
}

/* Search Box and Search Button */
void MainWindow::onSearchClicked()
{

}

/* Undo, Redo */
Snapshot MainWindow::takeSnapshot()
{
    Snapshot s;
    for (int r = 0; r < ui->invoiceTable->rowCount(); r++) {
        QStringList row;
        for (int c = 0; c < 4; c++)
            row << ui->invoiceTable->item(r, c)->text();
        s.append(row);
    }
    return s;
}

void MainWindow::restoreSnapshot(const Snapshot &s)
{
    ui->invoiceTable->setRowCount(0);
    for (const QStringList &row : s)
        addInvoiceRow(row[0], row[1], row[2].toInt(), row[3].toDouble());
    tempRecalculate();
}

void MainWindow::saveForUndo()
{
    undoStack.push(takeSnapshot());
    redoStack.clear();
}

void MainWindow::onUndoClicked()
{
    if (undoStack.isEmpty()) return;
    redoStack.push(takeSnapshot());
    restoreSnapshot(undoStack.pop());
}

void MainWindow::onRedoClicked()
{
    if (redoStack.isEmpty()) return;
    undoStack.push(takeSnapshot());
    restoreSnapshot(redoStack.pop());
}

/* Remove Item */
void MainWindow::onRemoveClicked()
{
    int row = ui->invoiceTable->currentRow();
    if (row < 0)
        return;

    saveForUndo();

    ui->invoiceTable->removeRow(row);
    tempRecalculate();
}

/* Sales Card */
void MainWindow::showSalesCards(double totalSales, const QString &bestName, int bestUnits,
                                const QString &leastName, int leastUnits)
{
    ui->totalSalesValue->setText(QString("\u20B1%1").arg(totalSales, 0, 'f', 2));
    ui->bestSellerName->setText(bestName);
    ui->bestSellerUnits->setText(QString("%1 units sold").arg(bestUnits));
    ui->leastSoldName->setText(leastName);
    ui->leastSoldUnits->setText(QString("%1 units sold").arg(leastUnits));
}

void MainWindow::addSalesRow(const QString &item, int units, const QString &status)
{
    int row = ui->salesTable->rowCount();
    ui->salesTable->insertRow(row);
    ui->salesTable->setItem(row, 0, new QTableWidgetItem(item));
    ui->salesTable->setItem(row, 1, new QTableWidgetItem(QString::number(units)));
    ui->salesTable->setItem(row, 2, new QTableWidgetItem(status));
}

void MainWindow::refreshSalesReport()
{
    ui->salesTable->setRowCount(0);
    showSalesCards(salesTotal, "-", 0, "-", 0);
}

void MainWindow::onAddClicked()
{
    saveForUndo();

    int i = sender()->property("row").toInt();

    QString name = ui->catalogTable->item(i, 1)->text();
    QString type = "Piece";
    QString price = ui->catalogTable->item(i, 2)->text();
    if (price == "-") {
        type = "Rolls";
        price = ui->catalogTable->item(i, 3)->text();
    }
    addInvoiceRow(name, type, 1, price.remove(",").toDouble());
    tempRecalculate();
}

MainWindow::~MainWindow()
{
    delete ui;
}

/* Page Switching */
void MainWindow::onMenuButtonClicked()
{
    ui->pagesStack->setCurrentIndex(0);
}

void MainWindow::onInvoiceButtonClicked()
{
    ui->pagesStack->setCurrentIndex(1);
}

void MainWindow::onSalesButtonClicked()
{
    ui->pagesStack->setCurrentIndex(2);
}

void MainWindow::onInventoryButtonClicked()
{
    ui->pagesStack->setCurrentIndex(3);
}

/* Invoice */
void MainWindow::addInvoiceRow(const QString &name, const QString &type, int qty, double lineTotal)
{
    int row = ui->invoiceTable->rowCount();
    ui->invoiceTable->insertRow(row);
    ui->invoiceTable->setItem(row, 0, new QTableWidgetItem(name));
    ui->invoiceTable->setItem(row, 1, new QTableWidgetItem(type));
    ui->invoiceTable->setItem(row, 2, new QTableWidgetItem(QString::number(qty)));
    ui->invoiceTable->setItem(row, 3, new QTableWidgetItem(QString::number(lineTotal, 'f', 2)));
}

void MainWindow::showTotals(double subtotal, double tax, double total)
{
    ui->subtotalLabel->setText(QString("Subtotal: \u20B1%1").arg(subtotal, 0, 'f', 2));
    ui->taxLabel->setText(QString("Tax (12%): \u20B1%1").arg(tax, 0, 'f', 2));
    ui->totalLabel->setText(QString("Total: \u20B1%1").arg(total, 0, 'f', 2));
}

void MainWindow::tempRecalculate()
{
    showTotals(0, 0, 0);
}