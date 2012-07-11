#ifndef QC_H
#define QC_H

#include "otwo/o1.h"

/// Quantis Cloud connector and uploader.
class Qc : public O1 {
    Q_OBJECT

public:
    explicit Qc(QObject *parent = 0);
    ~Qc();

    /// Is uploading enabled?
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    bool enabled();
    void setEnabled(bool v);

    static Qc *instance();
    static void close();

    /// Start uploading some records from an archive.
    void upload(const QString &archive);

signals:
    void enabledChanged();

public slots:

};

#endif // QC_H
