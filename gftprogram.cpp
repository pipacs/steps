#include <QDebug>
#include <QNetworkAccessManager>

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
    step();
    exec();
}

void GftProgram::step() {
    qDebug() << "GftProgram::step";

    if (status == Completed || status == Failed) {
        qCritical() << "GftProgram::step: Attempting to run a Completed or Failed program";
        emit completed(status, error, errorMsg);
        quit();
        return;
    }

    if (status == Idle) {
        status = Running;
    }

    if (error != NoError) {
        qDebug() << " Error" << error << errorMsg;
        status = Failed;
        emit completed(status, error, errorMsg);
        quit();
        return;
    }

    if (ic >= instructions.length()) {
        qDebug() << " No more instructions";
        status = (error == NoError)? Completed: Failed;
        emit completed(status, error, errorMsg);
        quit();
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
            qDebug() << " GftCreateTableIf --> Table exists, skipping";
            stepDone();
            return;
        }
        sql = QString("CREATE TABLE '%1' (steps: NUMBER, date: STRING, tags: STRING)").arg(instructions[ic].param);
        method = GftPost;
        break;

    default:
        // At this point, we should have a table ID. If there isn't, fail.
        if (tableId.isNull()) {
            qCritical() << "GftProgram::step: No table ID";
            status = Failed;
            error = SqlError;
            errorMsg = "No table ID";
            emit completed(status, error, errorMsg);
            quit();
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
        status = Failed;
        error = NetworkError;
        errorMsg = "Not logged in";
        emit completed(status, error, errorMsg);
        quit();
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

    QByteArray data = reply->readAll();
    qDebug() << "" << data;

    // FIXME:
    error = SqlError;

    reply->deleteLater();
    ic++;
    step();
}
