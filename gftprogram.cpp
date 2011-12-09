#include <assert.h>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QStringList>
#include <QTimer>

#include "gftprogram.h"
#include "gft.h"

enum GftMethod {GftGet, GftPost};

GftProgram::GftProgram(QObject *parent): QThread(parent), ic(0), status(Idle), error(NoError) {
    qDebug() << "GftProgram::GftProgram";
    manager = new QNetworkAccessManager(this);
}

GftProgram::~GftProgram() {
    qDebug() << "GftProgram::~GftProgram";
}

void GftProgram::setInstructions(const QList<GftInstruction> instructions_) {
    instructions = instructions_;
}

void GftProgram::run() {
    qDebug() << "GftProgram::run";
    QTimer::singleShot(0, this, SLOT(step()));
    exec();
}

void GftProgram::step() {
    qDebug() << "GftProgram::step";

    if (status == Completed || status == Failed) {
        quit(status, error, "Attempting to run a Completed or Failed program");
        return;
    }

    if (status == Idle) {
        // End program if there was an error during execution
        status = Running;
    }

    if (error != NoError) {
        quit(Failed, error, errorMsg);
        return;
    }

    if (ic >= instructions.length()) {
        // No more instructions: End program
        quit((error == NoError)? Completed: Failed, error, errorMsg);
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
            // Table exists --> No need to create table
            ic++;
            QTimer::singleShot(0, this, SLOT(step()));
            return;
        }
        sql = QString("CREATE TABLE '%1' (steps: NUMBER, date: STRING, tags: STRING)").arg(instructions[ic].param);
        method = GftPost;
        break;

    default:
        // At this point, we should have a table ID. If there isn't, fail.
        if (tableId.isNull()) {
            quit(Failed, SqlError, "No table ID");
            return;
        }
        sql = instructions[ic].param;
        sql.replace("$T", tableId);
        method = GftPost;
    }

    qDebug() << "" << ((method == GftGet)? "GET": "POST") << sql;

    // Execute request

    Gft *gft = Gft::instance();
    if (!gft->linked()) {
        quit(Failed, NetworkError, "Not logged in");
        return;
    }
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
    reply = (method == GftGet)? manager->get(request): manager->post(request, data);
    connect(reply, SIGNAL(finished()), this, SLOT(stepDone()));
}

void GftProgram::stepDone() {
    qDebug() << "GftProgram::stepDone";

    if (reply->error() != QNetworkReply::NoError) {
        error = NetworkError;
        errorMsg = QString("Network error %1: %2").arg(reply->error()).arg(reply->errorString());
    } else {
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
                errorMsg = "Could not create table";
                error = SqlError;
            }
            break;

        default:
            if ((lines.length() >= 2) && (lines[0] == "rowid")) {
                lastRowId = lines[1];
            }
        }
    }

    reply->deleteLater();
    ic++;
    step();
}

void GftProgram::quit(Status status_, Error error_, const QString &errorMsg_) {
    status = status_;
    error = error_;
    errorMsg = errorMsg_;
    emit completed(status, error, errorMsg);
    QThread::quit();
}
