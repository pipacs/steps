#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QString>

/// SQLite database with on-demand initialization
class Database: public QObject {
    Q_OBJECT
public:
    explicit Database(const QString &name, QObject *parent = 0);
    ~Database();

    /// Get database, create database file if needed.
    QSqlDatabase &db();

signals:
    /// Emitted when database initialization is needed.
    void initialize();

protected:
    QSqlDatabase db_;
    QString name_;
};

#endif // DATABASE_H
