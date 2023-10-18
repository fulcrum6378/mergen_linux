#include "touchpad.h"

Touchpad::Touchpad(std::atomic_bool *on) : on_(on) {}

Touchpad::~Touchpad() = default;
