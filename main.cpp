#include "vis/camera.h"

int main() {
    auto *vis = new Camera();
    vis->Capture();
    delete vis;
    return 0;
}
