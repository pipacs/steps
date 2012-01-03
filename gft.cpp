#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDir>

#include "gftprogram.h"
#include "gft.h"
#include "gftsecret.h"
#include "database.h"
#include "trace.h"

static const char *GFT_OAUTH_SCOPE = "https://www.googleapis.com/auth/fusiontables";
static const char *GFT_OAUTH_ENDPOINT = "https://accounts.google.com/o/oauth2/auth";
static const char *GFT_OAUTH_TOKEN_URL = "https://accounts.google.com/o/oauth2/token";
static const char *GFT_OAUTH_REFRESH_TOKEN_URL = "https://accounts.google.com/o/oauth2/token";
const int GFT_RECORDS_PER_UPLOAD = 150;

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

Gft::Gft(QObject *parent): O2(GFT_OAUTH_CLIENT_ID, GFT_OAUTH_CLIENT_SECRET, GFT_OAUTH_SCOPE, QUrl(GFT_OAUTH_ENDPOINT), QUrl(GFT_OAUTH_TOKEN_URL), QUrl(GFT_OAUTH_REFRESH_TOKEN_URL), parent), program(0) {
}

Gft::~Gft() {
    delete program;
}

bool Gft::enabled() {
    return QSettings().value("gft.enabled", false).toBool();
}

void Gft::setEnabled(bool v) {
    QSettings().setValue("gft.enabled", v);
    emit enabledChanged();
}

void Gft::upload(const QString &archive_) {
    Trace t("Gft::upload");
    qDebug() << "Archive:" << archive_;

    // Create program if doesn't exist
    if (!program) {
        program = new GftProgram;
        connect(program, SIGNAL(stepCompleted(QList<qlonglong>)), this, SLOT(onStepCompleted(QList<qlonglong>)));
        connect(program, SIGNAL(programCompleted(bool)), this, SLOT(onProgramCompleted(bool)));
    }

    archive = archive_;
    uploadedRecords.clear();
    Database db(archive);

    // Do nothing if database is empty
    {
        qlonglong total = -1;
        QSqlQuery query("select count(*) from log", db.db());
        query.next();
        total = query.value(0).toLongLong();
        if (total == 0) {
            qDebug() << "Database empty";
            emit uploadFinished(UploadComplete);
            return;
        }
    }

    // Create Gft program's instructions

    QList<GftInstruction> instructions;

    QFileInfo info(archive);
    QString dbName = info.baseName();
    instructions.append(GftInstruction(GftFindTable, dbName));
    instructions.append(GftInstruction(GftCreateTableIf, dbName));

    QSqlQuery query(db.db());
    query.setForwardOnly(true);
    if (!query.exec("select id, date, steps from log")) {
        qCritical() << "Gft::upload: Could not query database:" << query.lastError().text();
        emit uploadFinished(UploadFailed);
        return;
    }
    int numRecords = 0;
    QString sql;
    QList<qlonglong> idList;
    while (query.next()) {
        if (++numRecords > GFT_RECORDS_PER_UPLOAD) {
            break;
        }
        qlonglong id = query.value(0).toLongLong();
        QString date = sanitize(query.value(1).toString().replace('T', ' '));
        int steps = query.value(2).toInt();
        QString tags = getTags(db, id);
        sql.append(QString("INSERT INTO $T (steps,date,tags) VALUES (%1,'%2','%3');\n").arg(steps).arg(date, tags));
        idList.append(id);
    }
    instructions.append(GftInstruction(GftQuery, sql, idList));

    // Execute Gft program
    qDebug() << " Executing GFT program";
    program->setInstructions(instructions);
    program->step();
}

QString Gft::getTags(Database &db, qlonglong id) {
    QString ret;
    QSqlQuery query(db.db());
    query.prepare("select name, value from tags where logId = ?");
    query.bindValue(0, id);
    if (!query.exec()) {
        qCritical() << "Gft::getTags: Could not query database:" << query.lastError().text();
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

void Gft::onStepCompleted(QList<qlonglong> recordIdList) {
    Trace t("Gft::onStepCompleted");
    foreach (qlonglong recordId, recordIdList) {
        uploadedRecords.append(recordId);
    }
}

void Gft::onProgramCompleted(bool failed) {
    Trace t("Gft::onProgramCompleted");
    int result = UploadFailed;

    // Delete uploaded records from local archive
    Database db(archive);
    db.transaction();
    foreach (qlonglong id, uploadedRecords) {
        QSqlQuery query(db.db());
        query.prepare("delete from log where id = ?");
        query.bindValue(0, id);
        query.exec();
    }
    db.commit();

    // Determine upload result
    if (failed) {
        qDebug() << "Result: Failed";
        result = UploadFailed;
    } else {
        // Are there any records left in the archive?
        qlonglong total = -1;
        QSqlQuery query("select count(*) from log", db.db());
        query.next();
        total = query.value(0).toLongLong();
        qDebug() << total << "records left";

        if (total == 0) {
            qDebug() << "Result: Complete";
            result = UploadComplete;
        } else {
            qDebug() << "Result: Incomplete";
            result = UploadIncomplete;
        }
    }
    emit uploadFinished(result);
}
