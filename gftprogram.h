/// Run SQL instructions on Google Fusion Tables

#ifndef GFTPROGRAM_H
#define GFTPROGRAM_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QThread>

/// POST URL for SQL queries.
#define GFT_SQL_URL "https://fusiontables.googleusercontent.com/fusiontables/api/query"
// #define GFT_SQL_URL "https://www.google.com/fusiontables/api/query"

class KQOAuthManager;
class KQOAuthRequest;

/// An operation.
enum GftOp {
    GftFindTable,       ///< Find table, return ID in GftProgram::tableId. Parameter is the table name.
    GftCreateTableIf,   ///< Create table if doesn't exist, return ID in GftProgram::tableId. Parameter is the table name.
    GftQuery            ///< Execute a query. Parameter is an SQL statement; $T will be replaced by the table ID.
};

/// Instruction: Operation and parameter
struct GftInstruction {
    GftInstruction(GftOp op_, const QString &param_): op(op_), param(param_) {}
    GftOp op;
    QString param;
};

/// Program
class GftProgram: public QThread {
    Q_OBJECT

public:
    enum Status {
        Idle,
        Running,
        Completed,
        Failed
    };

    GftProgram(QObject *parent = 0);
    ~GftProgram();
    void setToken(const QString &token);
    void setSecret(const QString &secret);
    void setInstructions(const QList<GftInstruction> instructions);
    void run();

public slots:
    /// Execute one instruction.
    void step();

    /// Request ready callback.
    void stepReady(QByteArray response);

    /// Request completed callback.
    void stepDone();

signals:
    /// Emitted when running the program is completed.
    void completed(Status status);

public:
    QString token;
    QString secret;
    QList<GftInstruction> instructions;
    int ic;
    Status status;
    QString tableId;
    QString errorMsg;
    KQOAuthManager *oauthManager;
    KQOAuthRequest *oauthRequest;
};

#endif // GFTPROGRAM_H
