#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDir>

#include "gftprogram.h"
#include "gft.h"
#include "gftsecret.h"

static const char *GFT_OAUTH_SCOPE = "https://www.googleapis.com/auth/fusiontables";
static const char *GFT_OAUTH_ENDPOINT = "https://accounts.google.com/o/oauth2/auth";
static const char *GFT_OAUTH_TOKEN_URL = "https://accounts.google.com/o/oauth2/token";
static const char *GFT_OAUTH_REFRESH_TOKEN_URL = "FIXME";

static Gft *instance_;

Gft *Gft::instance() {
    if (!instance_) {
        instance_ = new Gft();
    }
    return instance_;
}

void Gft::close() {
    delete instance_;
    instance_ = 0;
}

Gft::Gft(QObject *parent): O2(GFT_OAUTH_CLIENT_ID, GFT_OAUTH_CLIENT_SECRET, GFT_OAUTH_SCOPE, QUrl(GFT_OAUTH_ENDPOINT), QUrl(GFT_OAUTH_TOKEN_URL), QUrl(GFT_OAUTH_REFRESH_TOKEN_URL), parent) {
}

Gft::~Gft() {
}

bool Gft::enabled() {
    return QSettings().value("gft.enabled", false).toBool();
}

void Gft::setEnabled(bool v) {
    QSettings().setValue("gft.enabled", v);
    emit enabledChanged();
}

Gft::UploadResult Gft::upload(const QString &archive) {
    qDebug() << "Gft::upload" << archive;
    if (!enabled()) {
        // FIXME: This should be checked row by row
        qDebug() << " Not enabled";
        return Gft::UploadSucceeded;
    }
    if (!linked()) {
        qDebug() << " Not linked";
        return Gft::UploadFailed;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QDir::toNativeSeparators(archive));
    if (!db.open()) {
        qCritical() << "Gft::upload: Could not open database";
        return Gft::UploadFailed;
    }

    // Create Gft instruction list

    QList<GftInstruction> instructions;

    QFileInfo info(archive);
    QString dbName = info.baseName();
    instructions.append(GftInstruction(GftFindTable, dbName));
    instructions.append(GftInstruction(GftCreateTableIf, dbName));

    QSqlQuery query("select id, date, steps from log", db);
    query.setForwardOnly(true);
    if (!query.exec()) {
        qCritical() << "Gft::upload: Could not query database:" << query.lastError().text();
        return Gft::UploadFailed;
    }
    QSqlRecord record = query.record();
    int idIndex = record.indexOf("id");
    int dateIndex = record.indexOf("date");
    int stepsIndex = record.indexOf("steps");
    while (query.next()) {
        qlonglong id = query.value(idIndex).toLongLong();
        QString date = sanitize(query.value(dateIndex).toString());
        int steps = query.value(stepsIndex).toInt();
        QString tags = getTags(db, id);
        GftInstruction instruction(GftQuery, QString("INSERT INTO $T (steps, date, tags) VALUES (%1, '%2', '%3')").arg(steps).arg(date).arg(tags));
        qDebug() << "" << instruction.param;
        instructions.append(instruction);
    }
    db.close();

    // Execute Gft program
    qDebug() << " Running GFT program";
    GftProgram program(0);
    program.setInstructions(instructions);
    program.run();
    program.wait();
    qDebug() << " GFT program finished";

    return Gft::UploadFailed; // FIXME Gft::UploadSucceeded;
}

QString Gft::getTags(QSqlDatabase db, qlonglong id) {
    QString ret;
    QSqlQuery query("select name, value from tags where logId = ?", db);
    query.bindValue(0, id);
    if (!query.exec()) {
        qCritical() << "Gft::upload: Could not query database:" << query.lastError().text();
        return ret;
    }
    QSqlRecord record = query.record();
    int nameIndex = record.indexOf("name");
    int valueIndex = record.indexOf("value");
    while (query.next()) {
        QString name = query.value(nameIndex).toString();
        QString value = query.value(valueIndex).toString();
        ret.append(sanitize(name));
        ret.append("=");
        ret.append(sanitize(value));
        ret.append(";");
    }

    return ret;
}

QString Gft::sanitize(const QString &s) {
    QString ret = s;
    ret.remove('\'');
    ret.remove('"');
    ret.remove(';');
    ret.remove('=');
    ret.remove('\\');
    return ret;
}
