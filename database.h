#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QDebug>

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

    void close() {db_.close();}
    QSqlDatabase db() {initialize(); return db_;}
    bool transaction() {initialize(); return db_.transaction();}
    void commit() {initialize(); db_.commit();}
    void rollback() {initialize(); db_.rollback();}
    QString error() {return db_.lastError().text();}

signals:
    /// Emitted when adding the database schema is needed.
    /// An empty database has been created and opened at this point.
    void addSchema();

protected:
    /// Initialize database if needed.
    /// Upon first call, this will register a SQLite driver instance.
    /// The database will be created and opened if necessary.
    void initialize();

    QSqlDatabase db_;
    QString name_;
};

#endif // DATABASE_H
