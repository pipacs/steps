import QtQuick 1.1
import QtMultimediaKit 1.1

// A mutable sound effect
// Note: The "muted" propery has no effect on Symbian due to bug QTMOBILITY-865
SoundEffect {
    muted: prefs.muted
}
