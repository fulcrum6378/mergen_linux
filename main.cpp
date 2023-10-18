#include <chrono>
#include <iostream>
#include <thread>

#include "aud/microphone.h"
#include "hpt/touchpad.h"
#include "vis/camera.h"

using namespace std::chrono_literals;

static bool active = true;

void print(std::string s) {
    //std::system("clear");
    if (!s.empty()) std::cout << "\r" << s.c_str() << std::endl;
    std::cout << "Your command: ";
    std::cout.flush();
}

void work() {
    while (active) {
        print("Waiting for the miracle...");
        std::this_thread::sleep_for(2000ms);
    }
}

int main() {
    // construct interactions
    /*auto *vis = new Camera();
    if (vis->exit != 0) return vis->exit;
    auto *aud = new Microphone();
    if (aud->exit != 0) return aud->exit;
    auto *hpt = new Touchpad();
    if (hpt->exit != 0) return hpt->exit;

    // destruct interactions
    delete hpt;
    delete aud;
    delete vis;*/

    auto w = std::thread(&work);
    w.detach();
    int x;
    print("");
    while (active) {
        std::cin >> x;
        if (x != 0) print("");
        else {
            active = false;
            break;
        }
    }

    return 0;
}
