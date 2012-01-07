/// Run SQL instructions on Google Fusion Tables

#ifndef GFTPROGRAM_H
#define GFTPROGRAM_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QMetaType>

/// POST URL for SQL queries.
#define GFT_SQL_URL "https://fusiontables.googleusercontent.com/fusiontables/api/query"
// #define GFT_SQL_URL "https://www.google.com/fusiontables/api/query"

class QNetworkAccessManager;
class QNetworkReply;

/// List of record IDs.
typedef QList<qlonglong> GftIdList;
Q_DECLARE_METATYPE(GftIdList)

/// Operation.
enum GftOp {
    GftFindTable,       ///< Find table, return its ID in GftProgram::tableId. Parameter is the table name.
    GftCreateTableIf,   ///< Create table if doesn't exist, return its ID in GftProgram::tableId. Parameter is the table name.
    GftQuery            ///< Execute queries. Parameter is one or more SQL statements; $T will be replaced by the table ID.
};

/// Instruction.
struct GftInstruction {
    GftInstruction(GftOp op_, const QString &param_, QList<qlonglong> idList_ = QList<qlonglong>()): op(op_), param(param_), idList(idList_) {}
    GftOp op;           ///< Operation.
    QString param;      ///< Table name or SQL statements.
    GftIdList idList;   ///< Local record IDs to be uploaded by this instruction.
};

/// Upload program
class GftProgram: public QObject {
    Q_OBJECT

public:
    /// Execution status.
    enum Status {Idle, Running, Completed, Failed};

    explicit GftProgram(QObject *parent = 0);
    ~GftProgram();

    /// Set instructions, reset instruction counter and status.
    void setInstructions(const QList<GftInstruction> instructions_);

    /// Convert a local database name to GFT table name.
    QString toGftTableName(const QString &localName);

public slots:
    /// Execute next instruction.
    void step();

    /// Process finished request.
    void stepDone();

signals:
    /// Emitted when an instruction step is completed.
    /// @param  idList  Record IDs uploaded by to the step.
    void stepCompleted(GftIdList idList);

    /// Emitted when all instructions are completed.
    /// @param  failed  If true, execution had an error.
    void programCompleted(bool failed);

public:
    QList<GftInstruction> instructions;
    int ic;
    Status status;
    QString tableId;
    QNetworkAccessManager *manager;
    QNetworkReply *reply;
};

#endif // GFTPROGRAM_H
