#include <QtGlobal>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QSqlError>

#include "database.h"

Database::Database(const QString &name, QObject *parent): QObject(parent), name_(name) {
}

Database::~Database() {
    db_.close();
    if (db_.isValid()) {
        QSqlDatabase::removeDatabase(db_.connectionName());
    }
}

QSqlDatabase &Database::db() {
    bool schemaRequired = false;
    if (!db_.isValid()) {
        QString connectionName = QString::number(qrand());
        db_ = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    }

    if (db_.open()) {
        return db_;
    }

    // Create database
    QFileInfo dbInfo(name_);
    if (!dbInfo.exists()) {
        schemaRequired = true;
        QString dir = dbInfo.absolutePath();
        QFileInfo dirInfo(dir);
        if (!dirInfo.exists()) {
            if (!QDir().mkpath(dir)) {
                qCritical() << "Database::db: Failed to create" << dir;
                return db_;
            }
        } else if (!dirInfo.isDir()) {
            qCritical() << "Database::db:" << dir << "is not a directory";
            return db_;
        }
    }
    db_.setDatabaseName(QDir::toNativeSeparators(name_));
    if (!db_.open()) {
        qCritical() << "Database::db: Could not open database:" << db_.lastError().text();
        return db_;
    }

    if (schemaRequired) {
        emit addSchema();
    }
    return db_;
}
