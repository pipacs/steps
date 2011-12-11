#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QString>

/// SQLite database with on-demand initialization
class Database: public QObject {
    Q_OBJECT

public:
    /// Constructor.
    /// @param  name    Absolute database file name name.
    /// @param  parent  Parent object.
    explicit Database(const QString &name, QObject *parent = 0);

    /// Destructor.
    /// Close database and unregister the SQLite driver instance.
    ~Database();

    /// Get database.
    /// Upon first call, this will register a SQLite driver instance.
    /// The database will be created and opened if necessary.
    QSqlDatabase &db();

signals:
    /// Emitted when adding the database schema is needed.
    /// An empty database has been created and opened at this point.
    void addSchema();

protected:
    QSqlDatabase db_;
    QString name_;
};

#endif // DATABASE_H
