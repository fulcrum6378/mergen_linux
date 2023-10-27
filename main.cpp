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
    auto *visStm = new VisualSTM();

    // construct interactions
    auto *vis = new Camera(&on, visStm);
    if (vis->exit != 0) return vis->exit;
    auto *aud = new Microphone(&on);
    if (aud->exit != 0) return aud->exit;
    auto *hpt = new Touchpad(&on);
    if (hpt->exit != 0) return hpt->exit;

    // listen for a stop signal
    print("");
    std::cin.ignore();
    on = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(1200)); // unavoidable ensurance

    // destruct interactions
    delete hpt;
    delete aud;
    delete vis;

    // destruct higher-level components
    delete visStm;

    return 0;
}
