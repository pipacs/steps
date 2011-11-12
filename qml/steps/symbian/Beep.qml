import QtQuick 1.1
import QtMultimediaKit 1.1

// A mutable sound effect
SoundEffect {
    id: stepSound
    source: "file:///usr/share/sounds/ui-tones/snd_default_beep.wav"
    muted: prefs.muted
}
