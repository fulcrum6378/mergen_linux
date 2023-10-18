#include <iostream>
#include <thread>

#include "global.h"
#include "aud/microphone.h"
#include "hpt/touchpad.h"
#include "vis/camera.h"

int main() {
    static std::atomic_bool on = true;

    // construct high-level objects
    auto *visStm = new VisualSTM();

    // construct interactions
    auto *vis = new Camera(&on, visStm);
    if (vis->exit != 0) return vis->exit;
    auto *aud = new Microphone(&on);
    if (aud->exit != 0) return aud->exit;
    auto *hpt = new Touchpad(&on);
    if (hpt->exit != 0) return hpt->exit;

    // listen for commands
    int x;
    print("");
    while (on) {
        std::cin >> x;
        if (x != 0) print("");
        else {
            on = false;
            break;
        }
    }

    // destruct interactions
    delete hpt;
    delete aud;
    delete vis;

    // destruct high-level objects
    delete visStm;

    return 0;
}
