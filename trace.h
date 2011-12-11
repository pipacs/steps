#ifndef TRACE_H
#define TRACE_H

#include <QtGlobal>
#include <QtDebug>
#include <QString>
#include <QTime>
#include <QEvent>
#include <QFile>

#define TRACE Trace _(Q_FUNC_INFO)

/** Trace helper. */
class Trace {
public:
    Trace(const QString &s);
    ~Trace();
    static void messageHandler(QtMsgType type, const char *msg);
    static QtMsgType level;
    static void setFileName(const QString &fileName);
    static QString fileName();

protected:
    static QString prefix();
    QString name;
    static int indent;
    static QFile file;
};

#endif // TRACE_H
