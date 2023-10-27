#include <chrono>
#include <iostream>
#include <thread>

#include "global.hpp"
#include "aud/microphone.hpp"
#include "hpt/touchpad.hpp"
#include "vis/camera.hpp"
#include "vis/perception.hpp"

int main() {
    static std::atomic_bool on = true;

    // construct high-level components

    // construct interactions
    auto *vis2 = new Perception();
    if (vis2->exit != 0) return vis2->exit;
    auto *vis1 = new Camera(&on, vis2->stm);
    if (vis1->exit != 0) return vis1->exit;
    auto *aud = new Microphone(&on);
    if (aud->exit != 0) return aud->exit;
    auto *hpt = new Touchpad(&on);
    if (hpt->exit != 0) return hpt->exit;

    // listen for a stop signal
    print("");
    std::cin.ignore();
    on = false;
    std::this_thread::sleep_for(std::chrono::milliseconds(1500)); // unavoidable ensurance

    // destruct interactions
    delete hpt;
    delete aud;
    delete vis1;
    delete vis2;

    // destruct higher-level components

    return 0;
}
