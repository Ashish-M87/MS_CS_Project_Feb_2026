#ifndef ADD_EXPENSE_DIALOG_H
#define ADD_EXPENSE_DIALOG_H

#include "expense_record.h"
#include <QDialog>

namespace Ui { class AddExpenseDialog; }

/*
 * AddExpenseDialog — used for both adding and editing.
 *
 * Memory management to note:
 *   - m_ui is created with 'new' in the constructor.
 *   - m_ui is deleted with 'delete' in the destructor.
 *   - This is the most basic form of manual memory management in C++.
 *     Python does this automatically via reference counting.
 */
class AddExpenseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddExpenseDialog(QWidget* parent = NULL);
    ~AddExpenseDialog();   // destructor — deletes m_ui

    void          loadRecord(const ExpenseRecord& r);   // populate fields for editing
    ExpenseRecord getRecord() const;                    // read fields back out

private slots:
    void onSave();

private:
    bool validate();

    Ui::AddExpenseDialog* m_ui;   // raw pointer — we own it, we delete it
    int m_editId;
    int m_editUserId;
};

#endif // ADD_EXPENSE_DIALOG_H
