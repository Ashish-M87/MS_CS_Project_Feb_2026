#include "add_expense_dialog.h"
#include "ui_add_expense_dialog.h"
#include <QMessageBox>

AddExpenseDialog::AddExpenseDialog(QWidget* parent)
    : QDialog(parent)
    , m_ui(new Ui::AddExpenseDialog)   // allocate on the heap with 'new'
    , m_editId(-1)
    , m_editUserId(-1)
{
    m_ui->setupUi(this);
    setWindowTitle("Add Expense");

    m_ui->dateEdit->setDate(QDate::currentDate());
    m_ui->dateEdit->setCalendarPopup(true);
    m_ui->amountSpinBox->setMinimum(0.01);
    m_ui->amountSpinBox->setMaximum(999999.99);
    m_ui->amountSpinBox->setDecimals(2);
    m_ui->amountSpinBox->setPrefix("$ ");

    connect(m_ui->saveButton,   &QPushButton::clicked, this, &AddExpenseDialog::onSave);
    connect(m_ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

// Destructor — called automatically when the object is destroyed.
// We must delete m_ui here because we created it with 'new'.
// Python never needs this — the garbage collector handles it.
AddExpenseDialog::~AddExpenseDialog()
{
    delete m_ui;
}

void AddExpenseDialog::loadRecord(const ExpenseRecord& r)
{
    m_editId     = r.id;
    m_editUserId = r.userId;
    setWindowTitle("Edit Expense");
    m_ui->dateEdit->setDate(r.date);
    m_ui->amountSpinBox->setValue(r.amount);
    m_ui->categoryLineEdit->setText(r.category);
    m_ui->descriptionLineEdit->setText(r.description);
}

ExpenseRecord AddExpenseDialog::getRecord() const
{
    return ExpenseRecord(
        m_editId, m_editUserId,
        m_ui->dateEdit->date(),
        m_ui->amountSpinBox->value(),
        m_ui->categoryLineEdit->text().trimmed(),
        m_ui->descriptionLineEdit->text().trimmed());
}

void AddExpenseDialog::onSave()
{
    if (validate()) accept();
}

bool AddExpenseDialog::validate()
{
    if (m_ui->categoryLineEdit->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, "Validation", "Please enter a category.");
        return false;
    }
    if (m_ui->amountSpinBox->value() <= 0.0)
    {
        QMessageBox::warning(this, "Validation", "Amount must be greater than zero.");
        return false;
    }
    return true;
}
