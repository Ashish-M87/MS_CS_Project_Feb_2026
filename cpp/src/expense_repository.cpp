#include "expense_repository.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
//
// Called once when someone writes:  ExpenseRepository* repo = new ExpenseRepository(path);
// In Python:  repo = ExpenseRepository(path)   — same idea, different memory model.
// ─────────────────────────────────────────────────────────────────────────────

ExpenseRepository::ExpenseRepository(const QString& filePath)
    : m_filePath(filePath)
    , m_nextUserId(1)
    , m_nextExpenseId(1)
{
    load();   // read existing data from disk straight away
}

// ─────────────────────────────────────────────────────────────────────────────
// User management
// ─────────────────────────────────────────────────────────────────────────────

std::vector<UserRecord> ExpenseRepository::getUsers() const
{
    // Returns a COPY of the vector.
    // In Python: return list(self._users)  — also a copy.
    return m_users;
}

int ExpenseRepository::addUser(const QString& name)
{
    QString trimmed = name.trimmed();
    if (trimmed.isEmpty())
        return -1;

    // Check for duplicate name
    for (int i = 0; i < (int)m_users.size(); i++)
    {
        if (m_users[i].name.compare(trimmed, Qt::CaseInsensitive) == 0)
            return -1;
    }

    UserRecord u(m_nextUserId++, trimmed);
    m_users.push_back(u);   // Python: self._users.append(u)
    save();
    return u.id;
}

bool ExpenseRepository::removeUser(int id)
{
    for (int i = 0; i < (int)m_users.size(); i++)
    {
        if (m_users[i].id == id)
        {
            m_users.erase(m_users.begin() + i);   // Python: del self._users[i]
            save();
            return true;
        }
    }
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Expense CRUD
// ─────────────────────────────────────────────────────────────────────────────

int ExpenseRepository::addExpense(const ExpenseRecord& e)
{
    ExpenseRecord r = e;          // copy the struct (value semantics)
    r.id = m_nextExpenseId++;
    m_expenses.push_back(r);
    save();
    return r.id;
}

bool ExpenseRepository::updateExpense(const ExpenseRecord& updated)
{
    for (int i = 0; i < (int)m_expenses.size(); i++)
    {
        if (m_expenses[i].id == updated.id)
        {
            m_expenses[i] = updated;   // overwrite the slot
            save();
            return true;
        }
    }
    return false;
}

bool ExpenseRepository::deleteExpense(int id)
{
    for (int i = 0; i < (int)m_expenses.size(); i++)
    {
        if (m_expenses[i].id == id)
        {
            m_expenses.erase(m_expenses.begin() + i);
            save();
            return true;
        }
    }
    return false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Filtered query — builds and returns a filtered list
//
// Python equivalent:
//     def get_expenses(self, user_id, from_date, to_date):
//         return [e for e in self._expenses
//                 if e['userId'] == user_id
//                 and from_date <= e['date'] <= to_date]
// ─────────────────────────────────────────────────────────────────────────────

std::vector<ExpenseRecord> ExpenseRepository::getExpenses(
    int userId, const QDate& from, const QDate& to) const
{
    std::vector<ExpenseRecord> result;
    for (int i = 0; i < (int)m_expenses.size(); i++)
    {
        const ExpenseRecord& e = m_expenses[i];
        if (e.userId == userId && e.date >= from && e.date <= to)
            result.push_back(e);
    }
    return result;
}

// ─────────────────────────────────────────────────────────────────────────────
// Aggregation
// ─────────────────────────────────────────────────────────────────────────────

double ExpenseRepository::totalFor(const std::vector<ExpenseRecord>& records) const
{
    double total = 0.0;
    for (int i = 0; i < (int)records.size(); i++)
        total += records[i].amount;
    return total;
}

int ExpenseRepository::countFor(int userId) const
{
    int count = 0;
    for (int i = 0; i < (int)m_expenses.size(); i++)
    {
        if (m_expenses[i].userId == userId)
            count++;
    }
    return count;
}

// ─────────────────────────────────────────────────────────────────────────────
// JSON persistence
// ─────────────────────────────────────────────────────────────────────────────

void ExpenseRepository::save() const
{
    QJsonArray usersArr;
    for (int i = 0; i < (int)m_users.size(); i++)
    {
        QJsonObject o;
        o["id"]   = m_users[i].id;
        o["name"] = m_users[i].name;
        usersArr.append(o);
    }

    QJsonArray expArr;
    for (int i = 0; i < (int)m_expenses.size(); i++)
    {
        const ExpenseRecord& e = m_expenses[i];
        QJsonObject o;
        o["id"]          = e.id;
        o["userId"]      = e.userId;
        o["date"]        = e.date.toString(Qt::ISODate);
        o["amount"]      = e.amount;
        o["category"]    = e.category;
        o["description"] = e.description;
        expArr.append(o);
    }

    QJsonObject root;
    root["users"]    = usersArr;
    root["expenses"] = expArr;

    QFile file(m_filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
        file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
}

void ExpenseRepository::load()
{
    QFile file(m_filePath);
    if (!file.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();

    m_users.clear();
    m_nextUserId = 1;
    QJsonArray usersArr = root["users"].toArray();
    for (int i = 0; i < usersArr.size(); i++)
    {
        QJsonObject o = usersArr[i].toObject();
        UserRecord u(o["id"].toInt(), o["name"].toString());
        if (u.isValid())
        {
            m_users.push_back(u);
            if (u.id >= m_nextUserId) m_nextUserId = u.id + 1;
        }
    }

    m_expenses.clear();
    m_nextExpenseId = 1;
    QJsonArray expArr = root["expenses"].toArray();
    for (int i = 0; i < expArr.size(); i++)
    {
        QJsonObject o = expArr[i].toObject();
        ExpenseRecord e;
        e.id          = o["id"].toInt();
        e.userId      = o["userId"].toInt(-1);
        e.date        = QDate::fromString(o["date"].toString(), Qt::ISODate);
        e.amount      = o["amount"].toDouble();
        e.category    = o["category"].toString();
        e.description = o["description"].toString();

        // Assign to first user if old record has no userId
        if (e.userId < 0 && !m_users.empty())
            e.userId = m_users[0].id;

        if (e.isValid())
        {
            m_expenses.push_back(e);
            if (e.id >= m_nextExpenseId) m_nextExpenseId = e.id + 1;
        }
    }
}
