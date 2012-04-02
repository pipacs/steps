#include <assert.h>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QStringList>
#include <QTimer>

#include "gftprogram.h"
#include "gft.h"
#include "trace.h"

enum GftMethod {GftGet, GftPost};

GftProgram::GftProgram(QObject *parent): QObject(parent) , requestId(-1) {
    qRegisterMetaType<GftIdList>();
    manager = new QNetworkAccessManager(this);
    requestor = new O2Requestor(manager, Gft::instance(), this);
    connect(requestor, SIGNAL(finished(int,QNetworkReply::NetworkError,QByteArray)), this, SLOT(stepDone(int,QNetworkReply::NetworkError,QByteArray)));
}

GftProgram::~GftProgram() {
}

void GftProgram::setInstructions(const QList<GftInstruction> instructions_) {
    instructions = instructions_;
    ic = 0;
    status = Idle;
}

void GftProgram::step() {
    Trace t("GftProgram::step");

    assert(status != Completed);

    if (status == Failed) {
        qDebug() << "Failed";
        emit programCompleted(true);
        return;
    }

    if (ic >= instructions.length()) {
        qDebug() << "Completed";
        emit programCompleted(false);
        return;
    }

    if (status == Idle) {
        qDebug() << "Idle -> Running";
        status = Running;
    }

    Gft *gft = Gft::instance();
    if (!gft->enabled()) {
        qDebug() << "Not enabled";
        emit programCompleted(false);
        return;
    }
    if (!gft->linked()) {
        qDebug() << "Not logged in";
        emit programCompleted(false);
        return;
    }

    // Build SQL statement based on current instruction

    QString sql;
    GftMethod method;

    switch (instructions[ic].op) {
    case GftFindTable:
        sql = QString("SHOW TABLES");
        method = GftGet;
        break;

    case GftCreateTableIf:
        if (!tableId.isNull()) {
            // Table exists, so there is no need to create table. Execute next step instead
            ic++;
            QTimer::singleShot(0, this, SLOT(step()));
            return;
        }
        sql = QString("CREATE TABLE '%1' (steps: NUMBER, date: DATETIME, tags: STRING, device: STRING)").arg(instructions[ic].param);
        method = GftPost;
        break;

    default:
        // At this point, we should have a table ID. If we don't, fail.
        if (tableId.isNull()) {
            qDebug() << "No table ID";
            emit programCompleted(true);
            return;
        }
        sql = instructions[ic].param;
        sql.replace("$T", tableId);
        method = GftPost;
    }

    // Execute request
    qDebug() << "Sending request to Google";
    QUrl url(GFT_SQL_URL);
    if (method == GftGet) {
        url.addQueryItem("sql", sql);
    }
    QNetworkRequest request(url);
    if (method == GftGet) {
        requestId = requestor->get(request);
    } else {
        QByteArray data;
        data.append("sql=");
        data.append(QUrl::toPercentEncoding(sql.toUtf8()));
        requestId = requestor->post(request, data);
    }
}

void GftProgram::stepDone(int id, QNetworkReply::NetworkError error, const QByteArray &data) {
    Trace t("GftProgram::stepDone");

    if (id != requestId) {
        qWarning() << "GftProgram::stepDone: Unknown request ID" << id;
        return;
    }

    if (error == QNetworkReply::NoError) {
        QStringList lines = QString(data).split("\n");
        switch (instructions[ic].op) {
        case GftFindTable: {
            QString gftName = instructions[ic].param;
            foreach (QString line, lines) {
                int commaIndex = line.indexOf(',');
                if (commaIndex >= 0) {
                    QString id = line.left(commaIndex);
                    QString name = line.mid(commaIndex + 1);
                    if (name == gftName) {
                        qDebug() << "Found fusion table, name:" << name << ", ID:" << id;
                        tableId = id;
                        break;
                    }
                }
            }
            break;
        }
        case GftCreateTableIf:
            if ((lines.length() >= 2) && (lines[0] == "tableid")) {
                tableId = lines[1];
                qDebug() << "Created fusion table, ID:" << tableId;
            } else {
                status = Failed;
                qCritical() << "GftProgram::stepDone: Could not create fusion table";
            }
            break;
        default:
            emit stepCompleted(instructions[ic].idList);
        }
    } else {
        qCritical() << "GftProgram::stepDone: Error" << error;
        status = Failed;
    }

    ic++;
    QTimer::singleShot(0, this, SLOT(step()));
}
