#include <iostream>

#include "global.hpp"
#include "aud/microphone.hpp"
#include "aud/speaker.hpp"
#include "hpt/touchpad.hpp"
#include "vis/camera.hpp"

int main() {
    // construct low-level components (sensors/controls)
    int exit = 0;
    auto *aud_in = new Microphone(&exit);
    if (exit != 0) return 10 + exit;
    auto *aud_out = new Speaker(&exit);
    if (exit != 0) return 20 + exit;
    auto *hpt = new Touchpad(&exit);
    if (exit != 0) return 30 + exit;
    auto vis = new Camera(&exit);
    if (exit != 0) return 40 + exit;

    // listen for a stop signal
    print("");
    std::cin.ignore();
    on = false;

    // destruct low-level components
    delete aud_in;
    delete aud_out;
    delete hpt;
    delete vis;

    return 0;
}
