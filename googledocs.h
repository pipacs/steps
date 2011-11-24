#ifndef GOOGLEDOCS_H
#define GOOGLEDOCS_H

#include <QObject>

class GoogleDocs: public QObject {
    Q_OBJECT
    Q_PROPERTY(bool linked READ linked WRITE setLinked NOTIFY linkedChanged)

public:
    static GoogleDocs *instance();
    static void close();

    bool linked();
    void setLinked(bool v);

signals:
    void linkedChanged();

public slots:

protected:
    explicit GoogleDocs(QObject *parent = 0);
    virtual ~GoogleDocs();
};

#endif // GOOGLEDOCS_H
