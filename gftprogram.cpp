#include <assert.h>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QStringList>
#include <QTimer>

#include "gftprogram.h"
#include "gft.h"

enum GftMethod {GftGet, GftPost};

GftProgram::GftProgram(QObject *parent): QObject(parent) {
    manager = new QNetworkAccessManager;
}

GftProgram::~GftProgram() {
    delete manager;
}

void GftProgram::setInstructions(const QList<GftInstruction> instructions_) {
    instructions = instructions_;
    ic = 0;
    status = Idle;
}

void GftProgram::step() {
    qDebug() << "GftProgram::step";

    assert(status != Completed);

    if (status == Failed) {
        qDebug() << " Failed";
        emit programCompleted();
        return;
    }

    if (ic >= instructions.length()) {
        qDebug() << " Completed";
        emit programCompleted();
        return;
    }

    if (status == Idle) {
        status = Running;
    }

    Gft *gft = Gft::instance();
    if (!gft->enabled()) {
        qDebug() << " Upload disabled";
        emit programCompleted();
        return;
    }
    if (!gft->linked()) {
        qDebug() << " Not logged in";
        emit programCompleted();
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
        sql = QString("CREATE TABLE '%1' (steps: NUMBER, date: STRING, tags: STRING)").arg(instructions[ic].param);
        method = GftPost;
        break;

    default:
        // At this point, we should have a table ID. If we don't, fail.
        if (tableId.isNull()) {
            qDebug() << "GftProgram::step: No table ID";
            emit programCompleted();
            return;
        }
        sql = instructions[ic].param;
        sql.replace("$T", tableId);
        method = GftPost;
    }

    // Execute request
    QUrl url(GFT_SQL_URL);
    QByteArray data;
    url.addQueryItem("access_token", gft->token());
    if (method == GftGet) {
        url.addQueryItem("sql", sql);
    } else {
        data.append("sql=");
        data.append(QUrl::toPercentEncoding(sql.toUtf8()));
    }
    QNetworkRequest request(url);
    qDebug() << " Request:" << sql;
    reply = (method == GftGet)? manager->get(request): manager->post(request, data);
    connect(reply, SIGNAL(finished()), this, SLOT(stepDone()));
}

void GftProgram::stepDone() {
    qDebug() << "GftProgram::stepDone";

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QStringList lines = QString(data).split("\n");
        qDebug() << "" << lines;

        switch (instructions[ic].op) {
        case GftFindTable:
            foreach (QString line, lines) {
                int commaIndex = line.indexOf(',');
                if (commaIndex >= 0) {
                    QString id = line.left(commaIndex);
                    QString name = line.mid(commaIndex + 1);
                    if (name == instructions[ic].param) {
                        qDebug() << " Found table" << name << ": id" << id;
                        tableId = id;
                        break;
                    }
                }
            }
            break;

        case GftCreateTableIf:
            if ((lines.length() >= 2) && (lines[0] == "tableid")) {
                tableId = lines[1];
            } else {
                status = Failed;
            }
            break;

        default:
            if ((lines.length() >= 2) && (lines[0] == "rowid")) {
                emit stepCompleted(instructions[ic].id);
            }
        }
    }

    reply->deleteLater();
    ic++;
    QTimer::singleShot(0, this, SLOT(step()));
}
