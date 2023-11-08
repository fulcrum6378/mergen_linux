#include <chrono>
#include <iostream>
#include <thread>

#include "global.hpp"
#include "aud/microphone.hpp"
#include "aud/speaker.hpp"
#include "hpt/touchpad.hpp"
#include "vis/camera.hpp"

int main() {
    // construct low-level components (sensors/controls)
    auto *aud_in = new Microphone();
    if (aud_in->exit != 0) return aud_in->exit;
    auto *aud_out = new Speaker();
    if (aud_out->exit != 0) return aud_out->exit;
    auto *hpt = new Touchpad();
    if (hpt->exit != 0) return hpt->exit;
    auto *vis = new Camera();
    if (vis->exit != 0) return vis->exit;

    // listen for a stop signal
    print("");
    std::cin.ignore();
    on = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // unavoidable ensurance

    // destruct low-level components
    delete aud_in;
    delete aud_out;
    delete hpt;
    delete vis;

    return 0;
}
