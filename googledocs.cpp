#include "googledocs.h"

static GoogleDocs *instance_;

GoogleDocs *GoogleDocs::instance() {
    if (!instance) {
        instance_ = new GoogleDocs();
    }
    return instance_;
}

void GoogleDocs::close() {
    delete instance_;
    instance_ = 0;
}

GoogleDocs::GoogleDocs(QObject *parent): QObject(parent) {
}

GoogleDocs::~GoogleDocs() {
}

bool GoogleDocs::linked() {
    return false; // FIXME
}

void GoogleDocs::setLinked(bool v) {
    // FIXME
}
