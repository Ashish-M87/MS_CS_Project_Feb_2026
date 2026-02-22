#include "main_window.h"
#include "ui_main_window.h"
#include "add_expense_dialog.h"

#include <QToolBar>
#include <QAction>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QStatusBar>
#include <QInputDialog>
#include <QApplication>
#include <QDate>
#include <QDateEdit>
#include <QPushButton>
#include <QStandardPaths>
#include <QDir>

// ─────────────────────────────────────────────────────────────────────────────
// dataFilePath() — resolves where to store expenses.json
//
// QStandardPaths::AppLocalDataLocation gives a per-user folder that survives
// rebuilds:
//   Windows:  C:\Users\<name>\AppData\Local\ExpenseTracker\ExpenseTracker\
//   macOS:    ~/Library/Application Support/ExpenseTracker/
//   Linux:    ~/.local/share/ExpenseTracker/
//
// On first run we copy the seed file from shared/data/ if it exists.
// ─────────────────────────────────────────────────────────────────────────────
static QString dataFilePath()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir().mkpath(dir); // create the folder if it does not exist
    QString dest = dir + "/expenses.json";

    // const QString dest = QCoreApplication::applicationDirPath() + "/../shared/data/expenses.json";

    // Seed from the project's shared/data/ on first run only
    if (!QFile::exists(dest))
    {
        QString seed = QCoreApplication::applicationDirPath() + "/../shared/data/expenses.json";
        if (QFile::exists(seed))
            QFile::copy(seed, dest);
    }
    return dest;
}

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_ui(new Ui::MainWindow), m_repo(new ExpenseRepository(dataFilePath())), m_model(new ExpenseTableModel), m_proxy(NULL), m_summary(NULL), m_userCombo(NULL)
{
    m_ui->setupUi(this);
    setMinimumSize(960, 620);

    setupToolbar();
    setupFilterBar();
    setupTableView();

    m_summary = new SummaryWidget(this);
    m_ui->summaryContainer->layout()->addWidget(m_summary);

    // Ensure at least one user exists
    std::vector<UserRecord> users = m_repo->getUsers();
    if (users.empty())
    {
        m_repo->addUser("Default User");
        users = m_repo->getUsers();
    }
    m_currentUser = users[0];

    refresh();
}

// ─────────────────────────────────────────────────────────────────────────────
// Destructor — delete everything we created with new
// m_proxy, m_summary, m_userCombo are Qt-parented — deleted automatically
// ─────────────────────────────────────────────────────────────────────────────

MainWindow::~MainWindow()
{
    delete m_model;
    delete m_repo;
    delete m_ui;
}

// ─────────────────────────────────────────────────────────────────────────────
// Setup helpers
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::setupToolbar()
{
    QToolBar *bar = addToolBar("Main");
    bar->setMovable(false);

    QAction *actAdd = bar->addAction("+ Add");
    QAction *actEdit = bar->addAction("Edit");
    QAction *actDelete = bar->addAction("Delete");
    bar->addSeparator();
    QAction *actExport = bar->addAction("Export CSV");
    bar->addSeparator();

    bar->addWidget(new QLabel("  User: ", bar));
    m_userCombo = new QComboBox(bar);
    m_userCombo->setMinimumWidth(150);
    bar->addWidget(m_userCombo);
    bar->addSeparator();
    QAction *actManage = bar->addAction("Manage Users");

    // Populate user switcher
    std::vector<UserRecord> users = m_repo->getUsers();
    for (int i = 0; i < (int)users.size(); i++)
        m_userCombo->addItem(users[i].name, users[i].id);

    connect(actAdd, &QAction::triggered, this, &MainWindow::onAdd);
    connect(actEdit, &QAction::triggered, this, &MainWindow::onEdit);
    connect(actDelete, &QAction::triggered, this, &MainWindow::onDelete);
    connect(actExport, &QAction::triggered, this, &MainWindow::onExportCsv);
    connect(actManage, &QAction::triggered, this, &MainWindow::onManageUsers);
    connect(m_userCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onUserChanged);
}

void MainWindow::setupFilterBar()
{
    m_ui->dateFromEdit->setDate(QDate(QDate::currentDate().year(), 1, 1));
    m_ui->dateToEdit->setDate(QDate::currentDate());
    m_ui->dateFromEdit->setCalendarPopup(true);
    m_ui->dateToEdit->setCalendarPopup(true);

    m_ui->categoryCombo->addItem("All Categories");

    connect(m_ui->dateFromEdit, &QDateEdit::dateChanged,
            this, &MainWindow::onFilterChanged);
    connect(m_ui->dateToEdit, &QDateEdit::dateChanged,
            this, &MainWindow::onFilterChanged);
    connect(m_ui->categoryCombo,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onFilterChanged);
    connect(m_ui->clearFiltersBtn, &QPushButton::clicked,
            this, &MainWindow::onClearFilters);
}

void MainWindow::setupTableView()
{
    m_proxy = new QSortFilterProxyModel(this);
    m_proxy->setSourceModel(m_model);

    m_ui->tableView->setModel(m_proxy);
    m_ui->tableView->setSortingEnabled(true);
    m_ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_ui->tableView->setAlternatingRowColors(true);
    m_ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_ui->tableView->horizontalHeader()->setSectionResizeMode(
        ExpenseTableModel::COL_DESCRIPTION, QHeaderView::Stretch);
    m_ui->tableView->setColumnWidth(ExpenseTableModel::COL_DATE, 110);
    m_ui->tableView->setColumnWidth(ExpenseTableModel::COL_AMOUNT, 90);
    m_ui->tableView->setColumnWidth(ExpenseTableModel::COL_CATEGORY, 130);

    connect(m_ui->tableView, &QTableView::doubleClicked, this, &MainWindow::onEdit);
}

// ─────────────────────────────────────────────────────────────────────────────
// refresh — the single place that reads data and updates every widget
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::refresh()
{
    if (m_currentUser.id < 0)
        return;

    QDate from = m_ui->dateFromEdit->date();
    QDate to = m_ui->dateToEdit->date();
    QString cat = m_ui->categoryCombo->currentText();

    // Get date-filtered records for this user
    std::vector<ExpenseRecord> records = m_repo->getExpenses(m_currentUser.id, from, to);

    // Apply optional category filter
    if (cat != "All Categories")
    {
        std::vector<ExpenseRecord> filtered;
        for (int i = 0; i < (int)records.size(); i++)
        {
            if (records[i].category.compare(cat, Qt::CaseInsensitive) == 0)
                filtered.push_back(records[i]);
        }
        records = filtered;
    }

    // Push records into the table
    m_model->setRecords(records);

    // Build category totals map from whatever is currently showing
    std::map<QString, double> totals;
    double overallTotal = 0.0;
    for (int i = 0; i < (int)records.size(); i++)
    {
        totals[records[i].category] += records[i].amount;
        overallTotal += records[i].amount;
    }

    // Update summary panel + pie chart
    m_summary->refreshData(m_currentUser.name, totals, overallTotal);

    // Rebuild category dropdown from ALL this user's expenses (not just filtered)
    QString prevCat = m_ui->categoryCombo->currentText();
    m_ui->categoryCombo->blockSignals(true);
    m_ui->categoryCombo->clear();
    m_ui->categoryCombo->addItem("All Categories");
    std::vector<ExpenseRecord> all = m_repo->getExpenses(
        m_currentUser.id, QDate(2000, 1, 1), QDate::currentDate());
    for (int i = 0; i < (int)all.size(); i++)
    {
        if (m_ui->categoryCombo->findText(all[i].category) < 0)
            m_ui->categoryCombo->addItem(all[i].category);
    }
    int idx = m_ui->categoryCombo->findText(prevCat);
    m_ui->categoryCombo->setCurrentIndex(idx >= 0 ? idx : 0);
    m_ui->categoryCombo->blockSignals(false);

    setWindowTitle(QString("Expense Tracker — %1").arg(m_currentUser.name));
    statusBar()->showMessage(
        QString("User: %1  |  %2 records  |  Total: $%3")
            .arg(m_currentUser.name)
            .arg(records.size())
            .arg(overallTotal, 0, 'f', 2));
}

// ─────────────────────────────────────────────────────────────────────────────
// selectedId
// ─────────────────────────────────────────────────────────────────────────────

int MainWindow::selectedId() const
{
    QModelIndexList sel = m_ui->tableView->selectionModel()->selectedRows();
    if (sel.isEmpty())
        return -1;
    QModelIndex src = m_proxy->mapToSource(sel.first());
    return m_model->data(src, Qt::UserRole).toInt();
}

// ─────────────────────────────────────────────────────────────────────────────
// Slots
// ─────────────────────────────────────────────────────────────────────────────

void MainWindow::onAdd()
{
    AddExpenseDialog *dlg = new AddExpenseDialog(this);
    if (dlg->exec() == QDialog::Accepted)
    {
        ExpenseRecord r = dlg->getRecord();
        r.userId = m_currentUser.id;
        m_repo->addExpense(r);
        refresh();
    }
    delete dlg;
}

void MainWindow::onEdit()
{
    if (selectedId() == -1)
    {
        QMessageBox::information(this, "No Selection", "Select a row to edit.");
        return;
    }
    QModelIndexList sel = m_ui->tableView->selectionModel()->selectedRows();
    QModelIndex src = m_proxy->mapToSource(sel.first());
    ExpenseRecord existing = m_model->recordAt(src.row());

    AddExpenseDialog *dlg = new AddExpenseDialog(this);
    dlg->loadRecord(existing);
    if (dlg->exec() == QDialog::Accepted)
    {
        ExpenseRecord updated = dlg->getRecord();
        updated.userId = m_currentUser.id;
        m_repo->updateExpense(updated);
        refresh();
    }
    delete dlg;
}

void MainWindow::onDelete()
{
    int id = selectedId();
    if (id == -1)
    {
        QMessageBox::information(this, "No Selection", "Select a row to delete.");
        return;
    }
    if (QMessageBox::question(this, "Confirm", "Delete this expense?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
        m_repo->deleteExpense(id);
        refresh();
    }
}

void MainWindow::onExportCsv()
{
    QString path = QFileDialog::getSaveFileName(
        this, "Export CSV", QDir::homePath() + "/expenses.csv", "CSV (*.csv)");
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Error", "Could not open file.");
        return;
    }
    QTextStream out(&file);
    out << "Date,Amount,Category,Description\n";

    std::vector<ExpenseRecord> records = m_repo->getExpenses(
        m_currentUser.id, m_ui->dateFromEdit->date(), m_ui->dateToEdit->date());

    for (int i = 0; i < (int)records.size(); i++)
    {
        QString desc = records[i].description;
        desc.replace('"', '\'');
        out << records[i].date.toString(Qt::ISODate) << ","
            << QString::number(records[i].amount, 'f', 2) << ","
            << records[i].category << ","
            << "\"" << desc << "\"\n";
    }
    statusBar()->showMessage(QString("Exported %1 records.").arg(records.size()), 4000);
}

void MainWindow::onFilterChanged() { refresh(); }

void MainWindow::onClearFilters()
{
    m_ui->dateFromEdit->blockSignals(true);
    m_ui->dateToEdit->blockSignals(true);
    m_ui->categoryCombo->blockSignals(true);

    m_ui->dateFromEdit->setDate(QDate(QDate::currentDate().year(), 1, 1));
    m_ui->dateToEdit->setDate(QDate::currentDate());
    m_ui->categoryCombo->setCurrentIndex(0);

    m_ui->dateFromEdit->blockSignals(false);
    m_ui->dateToEdit->blockSignals(false);
    m_ui->categoryCombo->blockSignals(false);

    refresh();
}

void MainWindow::onUserChanged(int index)
{
    if (index < 0)
        return;
    int userId = m_userCombo->itemData(index).toInt();
    if (userId == m_currentUser.id)
        return;

    std::vector<UserRecord> users = m_repo->getUsers();
    for (int i = 0; i < (int)users.size(); i++)
    {
        if (users[i].id == userId)
        {
            m_currentUser = users[i];
            break;
        }
    }
    refresh();
}

void MainWindow::onManageUsers()
{
    QStringList opts;
    opts << "Add New User" << "Delete Current User";
    bool ok = false;
    QString choice = QInputDialog::getItem(
        this, "Manage Users", "Action:", opts, 0, false, &ok);
    if (!ok)
        return;

    if (choice == "Add New User")
    {
        QString name = QInputDialog::getText(this, "Add User", "Name:").trimmed();
        if (name.isEmpty())
            return;
        int newId = m_repo->addUser(name);
        if (newId < 0)
        {
            QMessageBox::warning(this, "Duplicate", "That name already exists.");
            return;
        }
        m_userCombo->blockSignals(true);
        m_userCombo->clear();
        std::vector<UserRecord> users = m_repo->getUsers();
        for (int i = 0; i < (int)users.size(); i++)
        {
            m_userCombo->addItem(users[i].name, users[i].id);
            if (users[i].id == newId)
                m_currentUser = users[i];
        }
        m_userCombo->blockSignals(false);
    }
    else
    {
        std::vector<UserRecord> users = m_repo->getUsers();
        if ((int)users.size() <= 1)
        {
            QMessageBox::warning(this, "Error", "Cannot delete the only user.");
            return;
        }
        if (QMessageBox::question(this, "Delete",
                                  QString("Delete \"%1\"?").arg(m_currentUser.name),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            m_repo->removeUser(m_currentUser.id);
            std::vector<UserRecord> remaining = m_repo->getUsers();
            m_currentUser = remaining[0];
            m_userCombo->blockSignals(true);
            m_userCombo->clear();
            for (int i = 0; i < (int)remaining.size(); i++)
                m_userCombo->addItem(remaining[i].name, remaining[i].id);
            m_userCombo->blockSignals(false);
        }
    }
    refresh();
}
