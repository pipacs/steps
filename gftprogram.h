/// Run SQL instructions on Google Fusion Tables

#ifndef GFTPROGRAM_H
#define GFTPROGRAM_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QList>

/// POST URL for SQL queries.
#define GFT_SQL_URL "https://fusiontables.googleusercontent.com/fusiontables/api/query"
// #define GFT_SQL_URL "https://www.google.com/fusiontables/api/query"

class QNetworkAccessManager;
class QNetworkReply;

/// An operation.
enum GftOp {
    GftFindTable,       ///< Find table, return ID in GftProgram::tableId. Parameter is the table name.
    GftCreateTableIf,   ///< Create table if doesn't exist, return ID in GftProgram::tableId. Parameter is the table name.
    GftQuery            ///< Execute a query. Parameter is an SQL statement; $T will be replaced by the table ID.
};

/// Instruction.
struct GftInstruction {
    GftInstruction(GftOp op_, const QString &param_, qlonglong id_ = -1): op(op_), param(param_), id(id_) {}
    GftOp op;           ///< Operation.
    QString param;      ///< Table name or SQL statement.
    qlonglong id;       ///< Local record ID.
};

/// Program
class GftProgram: public QObject {
    Q_OBJECT

public:
    /// Execution status.
    enum Status {Idle, Running, Completed, Failed};

    explicit GftProgram(QObject *parent = 0);
    ~GftProgram();

    /// Set instructions, reset instruction counter and status.
    void setInstructions(const QList<GftInstruction> instructions_);

public slots:
    /// Execute next instruction.
    void step();

    /// Process finished request.
    void stepDone();

signals:
    /// Emitted when an instruction step is completed.
    /// @param  id      Record ID corresponding to the step.
    void stepCompleted(qlonglong id);

    /// Emitted when all instructions are completed.
    void programCompleted();

public:
    QList<GftInstruction> instructions;
    int ic;
    Status status;
    QString tableId;
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
};

#endif // GFTPROGRAM_H
