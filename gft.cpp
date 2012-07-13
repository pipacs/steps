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
#include "platform.h"

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

Gft::Gft(QObject *parent): O2Gft(parent), program(0) {
    setClientId(GFT_OAUTH_CLIENT_ID);
    setClientSecret(GFT_OAUTH_CLIENT_SECRET);
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
        connect(program, SIGNAL(stepCompleted(GftIdList)), this, SLOT(onStepCompleted(GftIdList)));
        connect(program, SIGNAL(programCompleted(bool)), this, SLOT(onProgramCompleted(bool)));
    }

    archive = archive_;
    uploadedRecords.clear();
    Database db(archive);

    // Do nothing if database is empty
    {
        qlonglong total = -1;
        QSqlQuery query("select count(*) from log where ingft = 0", db.db());
        query.next();
        total = query.value(0).toLongLong();
        if (total == 0) {
            qDebug() << "No records to upload";
            emit uploadFinished(archive, UploadComplete);
            return;
        }
    }

    // Mark all records as uploaded if GFT is not enabled, then succeed
    if (!enabled()) {
        qDebug() << "GFT not enabled";
        QSqlQuery query("update log set ingft = 1", db.db());
        query.exec();
        emit uploadFinished(archive, UploadComplete);
        return;
    }

    // Fail if not logged in
    if (!linked()) {
        qDebug() << "Not logged in";
        emit uploadFinished(archive, UploadFailed);
        return;
    }

    // Create Gft program's instructions

    QList<GftInstruction> instructions;

    instructions.append(GftInstruction(GftFindTable, "Steps"));
    instructions.append(GftInstruction(GftCreateTableIf, "Steps"));

    QSqlQuery query(db.db());
    query.setForwardOnly(true);
    if (!query.exec("select id, date, steps from log where ingft = 0")) {
        qCritical() << "Gft::upload: Could not query archive:" << query.lastError().text();
        emit uploadFinished(archive, UploadFailed);
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
        QString device = Platform::instance()->deviceId();
        sql.append(QString("INSERT INTO $T (steps,date,tags,device) VALUES (%1,'%2','%3','%4');\n").arg(steps).arg(date, tags, device));
        idList.append(id);
    }
    instructions.append(GftInstruction(GftQuery, sql, idList));

    // Execute Gft program
    program->setInstructions(instructions);
    program->step();
}

QString Gft::getTags(Database &db, qlonglong id) {
    QString ret;
    QSqlQuery query(db.db());
    query.prepare("select name, value from tags where logId = ?");
    query.bindValue(0, id);
    if (!query.exec()) {
        qCritical() << "Gft::getTags: Could not query archive:" << query.lastError().text();
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

void Gft::onStepCompleted(GftIdList recordIdList) {
    Trace t("Gft::onStepCompleted");
    foreach (qlonglong recordId, recordIdList) {
        uploadedRecords.append(recordId);
    }
}

void Gft::onProgramCompleted(bool failed) {
    Trace _("Gft::onProgramCompleted");
    int result = UploadFailed;

    // Mark uploaded records in the local archive
    Database db(archive);
    db.transaction();
    foreach (qlonglong id, uploadedRecords) {
        QSqlQuery query(db.db());
        query.prepare("update log set ingft = 1 where id = ?");
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
        QSqlQuery query("select count(*) from log where ingft = 0", db.db());
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
    emit uploadFinished(archive, result);
}
