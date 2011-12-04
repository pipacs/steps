/// Run SQL instructions on Google Fusion Tables

#ifndef GFTPROGRAM_H
#define GFTPROGRAM_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QList>

/// POST URL for SQL queries.
#define GFT_SQL_URL "https://www.google.com/fusiontables/api/query"

class KQOAuthManager;
class KQOAuthRequest;

/// An operation.
enum GftOp {
    FindTable,
    CreateTable,
    Insert
};

/// Instruction: Operation and parameter
struct GftInstruction {
    GftOp op;
    QString param;
};

/// Program
class GftProgram: public QObject {
    Q_OBJECT

public:
    enum Status {
        Idle,
        Running,
        Completed,
        Failed
    };

    GftProgram(QObject *parent, const QString &token, const QString &secret, const QList<GftInstruction> &program);
    ~GftProgram();

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
    QList<GftInstruction> program;
    int ic;
    Status status;
    QString tableId;
    QString errorMsg;
    KQOAuthManager *oauthManager;
    KQOAuthRequest *oauthRequest;
};

#endif // GFTPROGRAM_H
