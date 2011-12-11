#include <QtGlobal>
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QSqlError>

#include "database.h"
#include "trace.h"

Database::Database(const QString &name, QObject *parent): QObject(parent), name_(name) {
}

Database::~Database() {
    db_.close();
    if (db_.isValid()) {
        QSqlDatabase::removeDatabase(db_.connectionName());
    }
}

void Database::initialize() {
    bool schemaRequired = false;

    if (!db_.isValid()) {
        QString connectionName = QString::number(qrand());
        db_ = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    }

    if (db_.isOpen()) {
        return;
    }

    // Create database
    if (!QFile::exists(name_)) {
        schemaRequired = true;
        QDir().mkpath(QFileInfo(name_).absolutePath());
    }
    db_.setDatabaseName(QDir::toNativeSeparators(name_));
    if (!db_.open()) {
        qCritical() << "Database::db: Could not open database:" << db_.lastError().text();
        return;
    }

    if (schemaRequired) {
        emit addSchema();
    }
}
