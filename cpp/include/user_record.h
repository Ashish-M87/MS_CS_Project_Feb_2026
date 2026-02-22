#ifndef USER_RECORD_H
#define USER_RECORD_H

#include <QString>

/*
 * UserRecord — another plain data struct.
 * Just an id and a name.  In Python this would simply be a dict.
 */
struct UserRecord
{
    int     id;
    QString name;

    UserRecord() : id(-1) {}
    UserRecord(int id, const QString& name) : id(id), name(name) {}

    bool isValid() const { return id >= 0 && !name.trimmed().isEmpty(); }
};

#endif // USER_RECORD_H
