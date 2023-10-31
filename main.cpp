#include <chrono>
#include <iostream>
#include <thread>

#include "global.hpp"
#include "aud/microphone.hpp"
#include "hpt/touchpad.hpp"
#include "vis/camera.hpp"

int main() {
    static std::atomic_bool on = true;

    // construct high-level components

    // construct low-level components (sensors/controls)
    auto *aud_in = new Microphone(&on);
    if (aud_in->exit != 0) return aud_in->exit;
    auto *hpt = new Touchpad(&on);
    if (hpt->exit != 0) return hpt->exit;
    auto *vis = new Camera(&on);
    if (vis->exit != 0) return vis->exit;

    // listen for a stop signal
    print("");
    std::cin.ignore();
    on = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // unavoidable ensurance

    // destruct low-level components
    delete aud_in;
    delete hpt;
    delete vis;

    // destruct higher-level components

    return 0;
}
