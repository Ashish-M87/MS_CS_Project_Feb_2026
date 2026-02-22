#include "expense_table_model.h"
#include <QColor>

ExpenseTableModel::ExpenseTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{}

int ExpenseTableModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return (int)m_records.size();
}

int ExpenseTableModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) return 0;
    return COLUMN_COUNT;
}

QVariant ExpenseTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= (int)m_records.size())
        return QVariant();

    const ExpenseRecord& r = m_records[index.row()];

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
            case COL_DATE:        return r.date.toString("dd MMM yyyy");
            case COL_AMOUNT:      return QString("$%1").arg(r.amount, 0, 'f', 2);
            case COL_CATEGORY:    return r.category;
            case COL_DESCRIPTION: return r.description;
        }
    }
    if (role == Qt::TextAlignmentRole && index.column() == COL_AMOUNT)
        return int(Qt::AlignRight | Qt::AlignVCenter);
    if (role == Qt::ForegroundRole && index.column() == COL_AMOUNT && r.amount > 500.0)
        return QColor(Qt::red);
    if (role == Qt::UserRole)
        return r.id;

    return QVariant();
}

QVariant ExpenseTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return QVariant();
    if (orientation == Qt::Vertical) return section + 1;
    switch (section)
    {
        case COL_DATE:        return QString("Date");
        case COL_AMOUNT:      return QString("Amount");
        case COL_CATEGORY:    return QString("Category");
        case COL_DESCRIPTION: return QString("Description");
    }
    return QVariant();
}

Qt::ItemFlags ExpenseTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void ExpenseTableModel::setRecords(const std::vector<ExpenseRecord>& records)
{
    beginResetModel();
    m_records = records;
    endResetModel();
}

ExpenseRecord ExpenseTableModel::recordAt(int row) const
{
    if (row < 0 || row >= (int)m_records.size()) return ExpenseRecord();
    return m_records[row];
}
