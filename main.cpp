#include "aud/microphone.h"
#include "hpt/touchpad.h"
#include "vis/camera.h"

int main() {
    // construct interactions
    auto *vis = new Camera();
    if (vis->exit != 0) return vis->exit;
    auto *aud = new Microphone();
    if (aud->exit != 0) return aud->exit;
    auto *hpt = new Touchpad();
    if (hpt->exit != 0) return hpt->exit;

    // destruct interactions
    delete hpt;
    delete aud;
    delete vis;
    return 0;
}
