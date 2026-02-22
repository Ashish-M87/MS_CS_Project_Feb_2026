#ifndef EXPENSE_TABLE_MODEL_H
#define EXPENSE_TABLE_MODEL_H

#include "expense_record.h"
#include <QAbstractTableModel>
#include <vector>

/*
 * ExpenseTableModel — bridges data to the QTableView (MVC pattern).
 *
 * Inherits from QAbstractTableModel and overrides virtual functions.
 * This is how C++ inheritance differs from Python:
 *   - 'override' keyword makes the compiler check we spelled the name right.
 *   - The base class calls our functions through a vtable (function pointer
 *     table) — that is polymorphism without Python's duck-typing.
 */
class ExpenseTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column { COL_DATE = 0, COL_AMOUNT, COL_CATEGORY, COL_DESCRIPTION, COLUMN_COUNT };

    explicit ExpenseTableModel(QObject* parent = NULL);

    // Required overrides
    int           rowCount   (const QModelIndex& parent = QModelIndex()) const override;
    int           columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant      data       (const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant      headerData (int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags      (const QModelIndex& index) const override;

    // Called by MainWindow to push a fresh snapshot into the view
    void          setRecords(const std::vector<ExpenseRecord>& records);
    ExpenseRecord recordAt  (int row) const;

private:
    std::vector<ExpenseRecord> m_records;
};

#endif // EXPENSE_TABLE_MODEL_H
