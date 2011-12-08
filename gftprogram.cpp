#include <QDebug>

#include "gftprogram.h"
#include "gft.h"

enum GftMethod {GftGet, GftPost};

GftProgram::GftProgram(QObject *parent): QThread(parent), ic(0), status(Idle) {
    qDebug() << "GftProgram::GftProgram";
}

GftProgram::~GftProgram() {
    qDebug() << "GftProgram::~GftProgram";
}

void GftProgram::setToken(const QString &token_) {
    token = token_;
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
        quit();
        return;
    }

    if (status == Idle) {
        status = Running;
    }

    if (ic >= instructions.length()) {
        qDebug() << " No more instructions";
        status = Completed;
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
            stepReady("-");
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
            quit();
            return;
        }
        sql = instructions[ic].param;
        sql.replace("$T", tableId);
        method = GftPost;
    }

    qDebug() << "" << ((method == GftGet)? "GET": "POST") << sql;

    // FIXME: Execute request
}

void GftProgram::stepReady(QByteArray response) {
    qDebug() << "GftProgram::stepReady" << response;

    // FIXME: Interpret response, set status and errorMsg
}

void GftProgram::stepDone() {
    qDebug() << "GftProgram::stepDone";
    // FIXME: Delete request
    ic++;
    step();
}
