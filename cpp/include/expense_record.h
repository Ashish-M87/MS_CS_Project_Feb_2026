#ifndef EXPENSE_RECORD_H
#define EXPENSE_RECORD_H

#include <QString>
#include <QDate>

/*
 * ExpenseRecord — a plain data struct.
 *
 * In Python you would use a dict:
 *     record = {"date": "2026-01-01", "amount": 45.0, "category": "Food"}
 *
 * In C++ we define a struct with a fixed type for every field.
 * The compiler knows the exact size in memory at compile time.
 * No garbage collector — the struct lives on the stack and is freed
 * automatically when it goes out of scope.
 */
struct ExpenseRecord
{
    int     id;
    int     userId;
    QDate   date;
    double  amount;
    QString category;
    QString description;

    // Default constructor — gives every field a safe starting value.
    // Python does not need this; dict values default to None.
    ExpenseRecord() : id(-1), userId(-1), amount(0.0) {}

    // Parameterised constructor — initialise everything at once.
    ExpenseRecord(int id, int userId, const QDate& date, double amount,
                  const QString& category, const QString& description)
        : id(id), userId(userId), date(date), amount(amount),
          category(category), description(description) {}

    bool isValid() const
    {
        return id >= 0 && userId >= 0 && date.isValid()
               && amount >= 0.0 && !category.isEmpty();
    }
};

#endif // EXPENSE_RECORD_H
