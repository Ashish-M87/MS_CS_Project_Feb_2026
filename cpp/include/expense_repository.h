#ifndef EXPENSE_REPOSITORY_H
#define EXPENSE_REPOSITORY_H

#include "expense_record.h"
#include "user_record.h"

#include <vector>
#include <QString>
#include <QDate>

/*
 * ExpenseRepository — the Model / data layer.
 *
 * This class owns ALL the data.  Nothing outside this class touches the
 * raw vectors directly — that is encapsulation.
 *
 * Python equivalent would be a class that holds two lists:
 *     class ExpenseRepository:
 *         def __init__(self):
 *             self.users    = []
 *             self.expenses = []
 *
 * C++ differences to notice:
 *   - Private data: m_users and m_expenses cannot be read or written from
 *     outside the class.  Python has no true private — _name is convention.
 *   - The constructor is called once when the object is created.
 *   - std::vector<T> is like a Python list but typed: every element must
 *     be an ExpenseRecord.  Python lists hold anything.
 *   - const on a method means it does not modify the object.
 *     Python has no equivalent keyword.
 *   - const& on a parameter means "read this without copying it".
 *     Python always passes references to objects automatically.
 */
class ExpenseRepository
{
public:
    // Constructor — loads existing data from disk when the object is created
    explicit ExpenseRepository(const QString& filePath);

    // ── User management ───────────────────────────────────────────────────────
    std::vector<UserRecord> getUsers() const;
    int  addUser(const QString& name);   // returns new id, or -1 if duplicate
    bool removeUser(int id);

    // ── Expense CRUD ──────────────────────────────────────────────────────────
    int  addExpense(const ExpenseRecord& e);   // returns assigned id
    bool updateExpense(const ExpenseRecord& e);
    bool deleteExpense(int id);

    // ── Filtered query ────────────────────────────────────────────────────────
    // Returns only the expenses belonging to userId within the date range.
    std::vector<ExpenseRecord> getExpenses(int userId,
                                            const QDate& from,
                                            const QDate& to) const;

    // ── Aggregation ───────────────────────────────────────────────────────────
    double totalFor(const std::vector<ExpenseRecord>& records) const;
    int    countFor(int userId) const;

    // ── Persistence ───────────────────────────────────────────────────────────
    void save() const;   // writes JSON to disk
    void load();         // reads JSON from disk

private:
    /*
     * Private data — only this class can read or modify these.
     *
     * In Python you would write:
     *     self._users    = []
     *     self._expenses = []
     * but nothing stops external code accessing _users.
     *
     * Here the compiler enforces privacy completely.
     */
    std::vector<UserRecord>    m_users;
    std::vector<ExpenseRecord> m_expenses;
    QString                    m_filePath;
    int                        m_nextUserId;
    int                        m_nextExpenseId;
};

#endif // EXPENSE_REPOSITORY_H
